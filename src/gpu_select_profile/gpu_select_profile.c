#include <fcntl.h>
#include <linux/vt.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <envycontrol_utils.h>

#define OUT_FILE "/run/optimus-boot.txt"
#define KERNEL_LOGLEVEL_FILE "/proc/sys/kernel/printk"
#define IDLE_TIMEOUT_LOOPS_MAX 100 // 1 loop = 100ms

static const char *options[] = {"Current", "Integrated", "Hybrid", "Nvidia"};
#define OPTION_COUNT ((int)(sizeof(options) / sizeof(options[0])))

static int select_index = 0;
static struct termios original;
static char kernel_log_backup[64];
static int redirect_fd = -1;
static volatile sig_atomic_t got_stop_signal = 0;

static int idle_timeout_loops = IDLE_TIMEOUT_LOOPS_MAX;

const char *current_profile;

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

#define LINE_MAX_LEN 40
#define HEADER_LINES 2
#define FOOTER_LINES 2
#define TOTAL_LINES (HEADER_LINES + FOOTER_LINES + OPTION_COUNT)
#define LAST_BOX_LINE (TOTAL_LINES - 1)
static void create_box_render(char box[][LINE_MAX_LEN]) {
  snprintf(box[0], LINE_MAX_LEN, "Current profile %s", current_profile);
  snprintf(box[1], LINE_MAX_LEN, "Select profile:");

  char prefix;
  for (int i = 0; i < OPTION_COUNT; i++) {
    prefix = (i == select_index) ? '>' : ' ';
    snprintf(box[i + HEADER_LINES], LINE_MAX_LEN, "%c%s", prefix, options[i]);
  }

  int timeout_in_seconds = idle_timeout_loops / 10;
  snprintf(box[LAST_BOX_LINE], LINE_MAX_LEN, "Keeping current profile in %ds",
           timeout_in_seconds);
}
static void render(void) {
  printf("\033[H\033[J");
  char box[TOTAL_LINES][LINE_MAX_LEN];
  create_box_render(box);
  int loops_max = sizeof(box) / sizeof(box[0]);

  for (int i = 0; i < loops_max; i++) {
    printf(box[i], '\n');
  }
  fflush(stdout);
}

static void move(int delta) {
  select_index = (select_index + delta + OPTION_COUNT) % OPTION_COUNT;
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
  render();
  while (!got_stop_signal) {
    if (idle_timeout_loops > 0 && idle_timeout_loops % 25 == 0)
      render();
    int c = read_key();
    if (c == 0) {
      if (--idle_timeout_loops <= 0) {
        select_index = 0;
        break;
      }
      continue;
    }
    idle_timeout_loops = IDLE_TIMEOUT_LOOPS_MAX;

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

  current_profile = envycontrol_get_current_profile();

  printf("\033[?25l");
  render();

  run_menu();

  if (got_stop_signal)
    select_index = 0;

  restore_terminal_state();

  write_profile(options[select_index]);
  return 0;
}
