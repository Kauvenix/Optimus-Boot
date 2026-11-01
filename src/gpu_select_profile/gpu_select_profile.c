#include <envycontrol_utils.h>
#include <fcntl.h>
#include <linux/vt.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define OUT_FILE "/run/optimus-boot.txt"
#define KERNEL_LOGLEVEL_FILE "/proc/sys/kernel/printk"
#define IDLE_TIMEOUT_LOOPS 100 // 1 loop = 100ms

static const char *options[] = {"Current", "Integrated", "Hybrid", "Nvidia"};
static const int option_count = sizeof(options) / sizeof(options[0]);

static int select_index = 0;
static struct termios original;
static char kernel_log_backup[64];
static int redirect_fd = -1;
static volatile sig_atomic_t got_stop_signal = 0;

static void on_stop_signal(int sig) {
  (void)sig;
  got_stop_signal = 1;
}
static void setup_stop_signals(void) {
  struct sigaction sa = {0};
  sa.sa_handler = on_stop_signal;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
}
static void virtual_terminal_switch(int virtual_terminal_number) {
  if (ioctl(STDIN_FILENO, VT_ACTIVATE, virtual_terminal_number) == 0)
    ioctl(STDIN_FILENO, VT_WAITACTIVE, virtual_terminal_number);
}

static void enable_raw_input(void) {
  struct termios raw = original;
  raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1; // 100 ms
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

// (/dev/console) -> tty1
static void redirect_systemd_messages_to_tty1(void) {
  redirect_fd = open("/dev/tty1", O_RDWR | O_NOCTTY);
  if (redirect_fd >= 0 && ioctl(redirect_fd, TIOCCONS) != 0) {
    close(redirect_fd);
    redirect_fd = -1;
  }
}

static void stop_systemd_message_redirect(void) {
  if (redirect_fd >= 0) {
    close(redirect_fd);
    redirect_fd = -1;
  }
}

static void silence_kernel_messages(void) {
  FILE *f = fopen(KERNEL_LOGLEVEL_FILE, "r");
  if (f) {
    if (!fgets(kernel_log_backup, sizeof kernel_log_backup, f))
      kernel_log_backup[0] = '\0';
    fclose(f);
  }
  f = fopen(KERNEL_LOGLEVEL_FILE, "w");
  if (f) {
    fputs("1\n", f);
    fclose(f);
  }
}

static void restore_kernel_messages(void) {
  if (!kernel_log_backup[0])
    return;
  FILE *f = fopen(KERNEL_LOGLEVEL_FILE, "w");
  if (f) {
    fputs(kernel_log_backup, f);
    fclose(f);
  }
}

static void restore_terminal_state(void) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
  printf("\033[?25h\033[H\033[J");
  fflush(stdout);
  restore_kernel_messages();
  stop_systemd_message_redirect();
  virtual_terminal_switch(1);
}

static void render(void) {
  printf("\033[H\033[J");
  printf("┌──────────────────┐\n│ Use Up / Down    │\n│   Select:        │\n");
  for (int i = 0; i < option_count; i++)
    printf(select_index == i ? "│  > %-13s │\n" : "│    %-13s │\n", options[i]);
  printf("└──────────────────┘\n");
  fflush(stdout);
}

static void move(int delta) {
  select_index = (select_index + delta + option_count) % option_count;
  render();
}

static int read_key(void) {
  unsigned char c;
  return read(STDIN_FILENO, &c, 1) == 1 ? c : 0;
}

static int write_profile(const char *profile) {
  FILE *f = fopen(OUT_FILE, "w");
  if (!f)
    return -1;
  fprintf(f, "gpu_profile=%s\n", profile);
  return fclose(f);
}

static void run_menu(void) {
  int idle = 0;
  render();
  while (!got_stop_signal) {
    if (idle > 0 && idle % 25 == 0)
      render();
    int c = read_key();
    if (c == 0) {
      if (++idle >= IDLE_TIMEOUT_LOOPS) {
        select_index = 0;
        break;
      }
      continue;
    }
    idle = 0;

    if (c != '\033') {
      if (c == '\n' || c == '\r')
        break;
      continue;
    }

    if (read_key() != '[')
      continue;

    c = read_key();
    if (c == 'A')
      move(-1);
    else if (c == 'B')
      move(1);
  }
}

int main(void) {
  // check tty
  if (!isatty(STDIN_FILENO) || tcgetattr(STDIN_FILENO, &original) != 0) {
    write_profile(options[0]);
    return 0;
  }

  setup_stop_signals();
  enable_raw_input();

  virtual_terminal_switch(8);
  redirect_systemd_messages_to_tty1();
  silence_kernel_messages();

  printf("\033[?25l");
  render();

  run_menu();

  if (got_stop_signal)
    select_index = 0;

  restore_terminal_state();

  write_profile(options[select_index]);
  return 0;
}
