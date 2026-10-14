/**
 * nowinch - Terminal window resize (SIGWINCH) filter proxy for Termux & Android
 *
 * Prevents unnecessary full-screen conversation redraws and lag in interactive
 * CLI TUIs (such as Antigravity CLI 'agy') when the Android virtual keyboard
 * opens or closes, or when switching apps, while preserving:
 *  - Native terminal scrollback and touch swipe scrolling.
 *  - Proper re-wrapping on true orientation changes (portrait <-> landscape).
 *
 * MIT License
 * Copyright (c) 2026 Andy Rathaur
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pty.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <errno.h>

static int master_fd = -1;
static pid_t child_pid = -1;
static struct termios orig_termios;
static int raw_mode_active = 0;
static struct winsize pty_ws;

static void cleanup_terminal(void) {
    if (raw_mode_active) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_mode_active = 0;
    }
}

static void sigterm_handler(int sig) {
    if (child_pid > 0) {
        kill(child_pid, sig);
    }
    cleanup_terminal();
    _exit(128 + sig);
}

static void sigwinch_handler(int sig) {
    (void)sig;
    struct winsize current_ws;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &current_ws) == 0 && master_fd >= 0) {
        /*
         * If the column count changed (e.g. screen rotated between portrait and landscape),
         * we forward the resize to the child so lines re-wrap to the new width.
         *
         * If ONLY the row count changed (virtual keyboard show/hide, IME toggle, or app
         * switching in Termux), we silently drop the signal. This prevents the child
         * process from unnecessarily redrawing and scrolling the entire conversation history!
         */
        if (current_ws.ws_col != pty_ws.ws_col) {
            pty_ws = current_ws;
            ioctl(master_fd, TIOCSWINSZ, &pty_ws);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        fprintf(stderr, "Example: %s agy\n", argv[0]);
        return 1;
    }

    if (isatty(STDIN_FILENO)) {
        if (tcgetattr(STDIN_FILENO, &orig_termios) == 0) {
            atexit(cleanup_terminal);
        }
        if (ioctl(STDIN_FILENO, TIOCGWINSZ, &pty_ws) == -1) {
            pty_ws.ws_col = 80;
            pty_ws.ws_row = 35;
        }
    } else {
        pty_ws.ws_col = 80;
        pty_ws.ws_row = 35;
    }

    /*
     * Ensure adequate minimum rows even if launched while the virtual keyboard
     * is already active on screen.
     */
    if (pty_ws.ws_row < 24) {
        pty_ws.ws_row = 35;
    }

    child_pid = forkpty(&master_fd, NULL, NULL, &pty_ws);
    if (child_pid < 0) {
        perror("forkpty");
        return 1;
    }

    if (child_pid == 0) {
        execvp(argv[1], &argv[1]);
        perror("execvp");
        _exit(127);
    }

    signal(SIGWINCH, sigwinch_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGHUP, sigterm_handler);

    if (isatty(STDIN_FILENO)) {
        struct termios raw = orig_termios;
        cfmakeraw(&raw);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        raw_mode_active = 1;
    }

    char buf[8192];
    int exit_status = 0;

    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(master_fd, &rfds);

        int max_fd = (STDIN_FILENO > master_fd ? STDIN_FILENO : master_fd) + 1;
        int n = select(max_fd, &rfds, NULL, NULL, NULL);

        if (n < 0) {
            if (errno == EINTR) continue;
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            ssize_t r = read(STDIN_FILENO, buf, sizeof(buf));
            if (r <= 0) break;
            ssize_t w = 0;
            while (w < r) {
                ssize_t written = write(master_fd, buf + w, r - w);
                if (written <= 0) break;
                w += written;
            }
        }

        if (FD_ISSET(master_fd, &rfds)) {
            ssize_t r = read(master_fd, buf, sizeof(buf));
            /* On Linux, read on master PTY returns -1 with EIO when slave closes */
            if (r <= 0) break;
            ssize_t w = 0;
            while (w < r) {
                ssize_t written = write(STDOUT_FILENO, buf + w, r - w);
                if (written <= 0) break;
                w += written;
            }
        }
    }

    cleanup_terminal();

    int status;
    waitpid(child_pid, &status, 0);
    if (WIFEXITED(status)) {
        exit_status = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        exit_status = 128 + WTERMSIG(status);
    }

    return exit_status;
}
