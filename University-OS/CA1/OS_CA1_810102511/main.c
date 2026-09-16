#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define GHOST_FILE ".ghost_session"
#define HISTORY_FILE ".chat_history"

#define MAX_TEXT 256
#define READ_BUF 512
#define HISTORY_BUF 32768

#define USER1_TYPE 1L
#define USER2_TYPE 2L

#define KIND_JOIN 1
#define KIND_READY 2
#define KIND_CHAT 3
#define KIND_BURN_NOTICE 4
#define KIND_EXIT 5

typedef struct {
    long mtype;
    int kind;
    int sender_id;
    int ttl;
    long message_id;
    long created_at;
    char mtext[MAX_TEXT];
} chat_message_t;

static int queue_id = -1;
static int is_user1 = 0;
static int local_user_id = 0;
static pid_t sender_pid = -1;
static pid_t receiver_pid = -1;
static pid_t timer_pid = -1;
static long message_counter = 1;

/* -------------------- Basic helpers -------------------- */

static size_t str_len(const char *text) {
    return strlen(text);
}

static void write_all(int fd, const char *buf, size_t len) {
    while (len > 0) {
        ssize_t written = write(fd, buf, len);
        if (written <= 0) {
            return;
        }
        buf += written;
        len -= (size_t)written;
    }
}

static void write_text(int fd, const char *text) {
    write_all(fd, text, str_len(text));
}

static void clear_screen(void) {
    write_all(STDOUT_FILENO, "\033[2J\033[H", 7);
}

/* Convert integer to text without stdio formatting functions. */
static void int_to_text(long value, char *out) {
    char temp[32];
    int index = 0;
    int negative = 0;

    if (value == 0) {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    if (value < 0) {
        negative = 1;
        value = -value;
    }

    while (value > 0 && index < (int)sizeof(temp) - 1) {
        temp[index++] = (char)('0' + (value % 10));
        value /= 10;
    }

    if (negative) {
        temp[index++] = '-';
    }

    for (int i = 0; i < index; ++i) {
        out[i] = temp[index - i - 1];
    }
    out[index] = '\0';
}

static void trim_newline(char *text) {
    size_t len = str_len(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[len - 1] = '\0';
        --len;
    }
}

static void sanitize_message(char *text) {
    for (int i = 0; text[i] != '\0'; ++i) {
        if (text[i] == '\n' || text[i] == '\r') {
            text[i] = '\0';
            return;
        }
        if (text[i] == '|') {
            text[i] = '/';
        }
    }
}

static int starts_with(const char *text, const char *prefix) {
    return strncmp(text, prefix, str_len(prefix)) == 0;
}

/* -------------------- History helpers -------------------- */

static int parse_number_field(const char *line, int *index, long *value) {
    int start = *index;
    int end = start;
    char number_text[32];
    int copy_len;

    while (line[end] != '\0' && line[end] != '|') {
        ++end;
    }

    if (line[end] != '|') {
        return 0;
    }

    copy_len = end - start;
    if (copy_len <= 0 || copy_len >= (int)sizeof(number_text)) {
        return 0;
    }

    memcpy(number_text, line + start, (size_t)copy_len);
    number_text[copy_len] = '\0';
    *value = strtol(number_text, NULL, 10);
    *index = end + 1;
    return 1;
}

static int parse_history_line(const char *line, long *message_id, long *created_at,
                              int *sender_id, int *ttl, char *text) {
    int index = 0;
    long temp = 0;

    if (!parse_number_field(line, &index, &temp)) {
        return 0;
    }
    *message_id = temp;

    if (!parse_number_field(line, &index, &temp)) {
        return 0;
    }
    *created_at = temp;

    if (!parse_number_field(line, &index, &temp)) {
        return 0;
    }
    *sender_id = (int)temp;

    if (!parse_number_field(line, &index, &temp)) {
        return 0;
    }
    *ttl = (int)temp;

    strncpy(text, line + index, MAX_TEXT - 1);
    text[MAX_TEXT - 1] = '\0';
    return 1;
}

static void append_history_line(char *buffer, int *offset, long message_id,
                                long created_at, int sender_id, int ttl,
                                const char *text) {
    char id_buf[32];
    char time_buf[32];
    char sender_buf[16];
    char ttl_buf[16];
    int needed;

    int_to_text(message_id, id_buf);
    int_to_text(created_at, time_buf);
    int_to_text(sender_id, sender_buf);
    int_to_text(ttl, ttl_buf);

    needed = (int)(str_len(id_buf) + str_len(time_buf) + str_len(sender_buf) +
                   str_len(ttl_buf) + str_len(text) + 5);

    if (*offset + needed >= HISTORY_BUF) {
        return;
    }

    memcpy(buffer + *offset, id_buf, str_len(id_buf));
    *offset += (int)str_len(id_buf);
    buffer[(*offset)++] = '|';

    memcpy(buffer + *offset, time_buf, str_len(time_buf));
    *offset += (int)str_len(time_buf);
    buffer[(*offset)++] = '|';

    memcpy(buffer + *offset, sender_buf, str_len(sender_buf));
    *offset += (int)str_len(sender_buf);
    buffer[(*offset)++] = '|';

    memcpy(buffer + *offset, ttl_buf, str_len(ttl_buf));
    *offset += (int)str_len(ttl_buf);
    buffer[(*offset)++] = '|';

    memcpy(buffer + *offset, text, str_len(text));
    *offset += (int)str_len(text);
    buffer[(*offset)++] = '\n';
    buffer[*offset] = '\0';
}

static void append_to_history(long message_id, long created_at, int sender_id,
                              int ttl, const char *text) {
    int fd = open(HISTORY_FILE, O_WRONLY | O_CREAT | O_APPEND, 0666);
    char line[512];
    int offset = 0;

    if (fd < 0) {
        return;
    }

    line[0] = '\0';
    append_history_line(line, &offset, message_id, created_at, sender_id, ttl, text);
    if (offset > 0) {
        write_all(fd, line, (size_t)offset);
    }

    close(fd);
}

static void print_header(void) {
    write_text(STDOUT_FILENO, "=== Ghost Chat ===\n");
    write_text(STDOUT_FILENO, "Commands: HISTORY, BURN <seconds> <message>, EXIT\n\n");
}

/* Show only messages that are still alive. */
static void show_history(void) {
    int fd = open(HISTORY_FILE, O_RDONLY);
    char buffer[HISTORY_BUF + 1];
    ssize_t bytes_read;
    time_t now = time(NULL);

    if (fd < 0) {
        write_text(STDOUT_FILENO, "No chat history yet.\n");
        return;
    }

    bytes_read = read(fd, buffer, HISTORY_BUF);
    close(fd);

    if (bytes_read <= 0) {
        write_text(STDOUT_FILENO, "No chat history yet.\n");
        return;
    }

    buffer[bytes_read] = '\0';
    write_text(STDOUT_FILENO, "\n=== Chat History ===\n");

    char *line = buffer;
    while (*line != '\0') {
        char *line_end = strchr(line, '\n');
        char saved = '\0';
        long message_id;
        long created_at;
        int sender_id;
        int ttl;
        char text[MAX_TEXT];
        char number_buf[16];

        if (line_end != NULL) {
            saved = *line_end;
            *line_end = '\0';
        }

        if (parse_history_line(line, &message_id, &created_at, &sender_id, &ttl, text)) {
            long age = (long)(now - created_at);

            if (ttl == 0 || age < ttl) {
                write_text(STDOUT_FILENO, "[User ");
                int_to_text(sender_id, number_buf);
                write_text(STDOUT_FILENO, number_buf);
                write_text(STDOUT_FILENO, "] ");
                write_text(STDOUT_FILENO, text);

                if (ttl > 0) {
                    write_text(STDOUT_FILENO, " (burns in ");
                    int_to_text(ttl - age, number_buf);
                    write_text(STDOUT_FILENO, number_buf);
                    write_text(STDOUT_FILENO, "s)");
                }

                write_text(STDOUT_FILENO, "\n");
            }
        }

        if (line_end == NULL) {
            break;
        }

        *line_end = saved;
        line = line_end + 1;
    }

    write_text(STDOUT_FILENO, "====================\n\n");
}

/* -------------------- Message queue helpers -------------------- */

static long next_message_id(void) {
    long now = (long)time(NULL);
    long pid_part = (long)(getpid() & 0x7FFF);
    long id = now * 100000L + pid_part * 100L + message_counter;

    ++message_counter;
    if (message_counter > 99) {
        message_counter = 1;
    }

    return id;
}

static void send_notice_to_both(const char *text) {
    chat_message_t notice;

    memset(&notice, 0, sizeof(notice));
    notice.kind = KIND_BURN_NOTICE;
    notice.sender_id = 0;
    notice.ttl = 0;
    strncpy(notice.mtext, text, MAX_TEXT - 1);

    notice.mtype = USER1_TYPE;
    msgsnd(queue_id, &notice, sizeof(notice) - sizeof(long), 0);

    notice.mtype = USER2_TYPE;
    msgsnd(queue_id, &notice, sizeof(notice) - sizeof(long), 0);
}

/* User 1 periodically removes expired burn messages from history. */
static void timer_manager_process(void) {
    while (1) {
        int fd;
        char buffer[HISTORY_BUF + 1];
        char survivors[HISTORY_BUF + 1];
        ssize_t bytes_read;
        int survivor_offset = 0;
        int changed = 0;
        time_t now;

        sleep(1);

        fd = open(HISTORY_FILE, O_RDONLY);
        if (fd < 0) {
            continue;
        }

        bytes_read = read(fd, buffer, HISTORY_BUF);
        close(fd);

        if (bytes_read <= 0) {
            continue;
        }

        buffer[bytes_read] = '\0';
        survivors[0] = '\0';
        now = time(NULL);

        char *line = buffer;
        while (*line != '\0') {
            char *line_end = strchr(line, '\n');
            char saved = '\0';
            long message_id;
            long created_at;
            int sender_id;
            int ttl;
            char text[MAX_TEXT];

            if (line_end != NULL) {
                saved = *line_end;
                *line_end = '\0';
            }

            if (parse_history_line(line, &message_id, &created_at, &sender_id, &ttl, text)) {
                if (ttl > 0 && (long)(now - created_at) >= ttl) {
                    changed = 1;
                    send_notice_to_both("[A burn message disappeared]");
                } else {
                    append_history_line(survivors, &survivor_offset, message_id,
                                        created_at, sender_id, ttl, text);
                }
            }

            if (line_end == NULL) {
                break;
            }

            *line_end = saved;
            line = line_end + 1;
        }

        if (changed) {
            fd = open(HISTORY_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd >= 0) {
                if (survivor_offset > 0) {
                    write_all(fd, survivors, (size_t)survivor_offset);
                }
                close(fd);
            }
        }
    }
}

/* -------------------- UI helpers -------------------- */

static void print_prompt(void) {
    write_text(STDOUT_FILENO, "You: ");
}

static void redraw_chat_screen(const char *notice) {
    clear_screen();
    print_header();

    if (notice != NULL && notice[0] != '\0') {
        write_text(STDOUT_FILENO, notice);
        write_text(STDOUT_FILENO, "\n\n");
    }

    show_history();
    print_prompt();
}

static void show_exit_notice(void) {
    clear_screen();
    print_header();
    write_text(STDOUT_FILENO, "[SYSTEM] The other user has left the chat\n");
}

/* -------------------- Child processes -------------------- */

static void receiver_process(void) {
    chat_message_t msg;
    char number_buf[16];

    while (1) {
        ssize_t result = msgrcv(queue_id, &msg, sizeof(msg) - sizeof(long),
                                (long)local_user_id, 0);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            write_text(STDERR_FILENO, "Receive failed.\n");
            _exit(1);
        }

        write_all(STDOUT_FILENO, "\r\033[K", 4);

        if (msg.kind == KIND_CHAT) {
            write_text(STDOUT_FILENO, "Stranger: ");
            write_text(STDOUT_FILENO, msg.mtext);

            if (msg.ttl > 0) {
                write_text(STDOUT_FILENO, " (burns in ");
                int_to_text(msg.ttl, number_buf);
                write_text(STDOUT_FILENO, number_buf);
                write_text(STDOUT_FILENO, "s)");
            }

            write_text(STDOUT_FILENO, "\n");
        } else if (msg.kind == KIND_BURN_NOTICE) {
            redraw_chat_screen(msg.mtext);
            continue;
        } else if (msg.kind == KIND_EXIT) {
            show_exit_notice();
            kill(getppid(), SIGUSR1);
            _exit(0);
        }

        print_prompt();
    }
}

/* Parse command: BURN <seconds> <message> */
static int parse_burn_command(char *input, int *ttl, char *message) {
    char *cursor = input + 5;
    char ttl_text[16];
    int ttl_len = 0;

    while (*cursor == ' ') {
        ++cursor;
    }

    while (*cursor >= '0' && *cursor <= '9' && ttl_len < (int)sizeof(ttl_text) - 1) {
        ttl_text[ttl_len++] = *cursor;
        ++cursor;
    }
    ttl_text[ttl_len] = '\0';

    while (*cursor == ' ') {
        ++cursor;
    }

    if (ttl_len == 0 || *cursor == '\0') {
        return 0;
    }

    *ttl = atoi(ttl_text);
    if (*ttl <= 0) {
        return 0;
    }

    strncpy(message, cursor, MAX_TEXT - 1);
    message[MAX_TEXT - 1] = '\0';
    sanitize_message(message);
    return message[0] != '\0';
}

static void send_chat_message(const char *text, int ttl) {
    chat_message_t msg;
    long created_at = (long)time(NULL);
    long message_id = next_message_id();

    memset(&msg, 0, sizeof(msg));
    if (strcmp(input, "HISTORY") == 0) {
    show_history();
    print_prompt();
    continue;
}
    msg.kind = KIND_CHAT;
    msg.sender_id = local_user_id;
    msg.ttl = ttl;
    msg.message_id = message_id;
    msg.created_at = created_at;
    strncpy(msg.mtext, text, MAX_TEXT - 1);

    if (msgsnd(queue_id, &msg, sizeof(msg) - sizeof(long), 0) < 0) {
        write_text(STDERR_FILENO, "Send failed.\n");
        return;
    }

    append_to_history(message_id, created_at, local_user_id, ttl, msg.mtext);
}

static void send_exit_message(void) {
    chat_message_t msg;

    memset(&msg, 0, sizeof(msg));
    msg.mtype = is_user1 ? USER2_TYPE : USER1_TYPE;
    msg.kind = KIND_EXIT;
    msg.sender_id = local_user_id;
    strncpy(msg.mtext, "EXIT", MAX_TEXT - 1);
    msgsnd(queue_id, &msg, sizeof(msg) - sizeof(long), 0);
}

static void sender_process(void) {
    char input[READ_BUF];

    print_prompt();
    while (1) {
        ssize_t n = read(STDIN_FILENO, input, sizeof(input) - 1);
        if (n <= 0) {
            continue;
        }

        input[n] = '\0';
        trim_newline(input);
        sanitize_message(input);

        if (input[0] == '\0') {
            print_prompt();
            continue;
        }

        if (strcmp(input, "HISTORY") == 0) {
            show_history();
            print_prompt();
            continue;
        }

        if (strcmp(input, "EXIT") == 0) {
            send_exit_message();
            kill(getppid(), SIGUSR1);
            _exit(0);
        }

        if (starts_with(input, "BURN ")) {
            int ttl = 0;
            char burn_text[MAX_TEXT];

            if (!parse_burn_command(input, &ttl, burn_text)) {
                write_text(STDOUT_FILENO, "Use: BURN <seconds> <message>\n");
                print_prompt();
                continue;
            }

            send_chat_message(burn_text, ttl);
            write_text(STDOUT_FILENO, "Burn message sent.\n");
            print_prompt();
            continue;
        }

        send_chat_message(input, 0);
        print_prompt();
    }
}

/* -------------------- Cleanup and setup -------------------- */

static void stop_child(pid_t pid) {
    if (pid > 0) {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
    }
}

static void cleanup_and_exit(int sig) {
    (void)sig;

    stop_child(sender_pid);
    stop_child(receiver_pid);
    stop_child(timer_pid);

    if (queue_id >= 0) {
        msgctl(queue_id, IPC_RMID, NULL);
    }

    unlink(GHOST_FILE);
    unlink(HISTORY_FILE);

    write_text(STDOUT_FILENO, "\nGoodbye!\n");
    _exit(0);
}

/* User 1 waits for JOIN, user 2 waits for READY. */
static int wait_for_ready_state(void) {
    chat_message_t msg;

    if (is_user1) {
        write_text(STDOUT_FILENO, "Waiting for User 2 to join...\n");

        while (1) {
            if (msgrcv(queue_id, &msg, sizeof(msg) - sizeof(long), USER1_TYPE, 0) < 0) {
                if (errno == EINTR) {
                    continue;
                }
                write_text(STDERR_FILENO, "Failed while waiting for User 2.\n");
                return 0;
            }

            if (msg.kind == KIND_JOIN) {
                write_text(STDOUT_FILENO, "User 2 connected. Chat is ready for both users.\n");
                memset(&msg, 0, sizeof(msg));
                msg.mtype = USER2_TYPE;
                msg.kind = KIND_READY;
                msg.sender_id = 1;
                strncpy(msg.mtext, "Chat is ready.", MAX_TEXT - 1);
                msgsnd(queue_id, &msg, sizeof(msg) - sizeof(long), 0);
                return 1;
            }
        }
    }

    memset(&msg, 0, sizeof(msg));
    msg.mtype = USER1_TYPE;
    msg.kind = KIND_JOIN;
    msg.sender_id = 2;
    strncpy(msg.mtext, "User 2 joined.", MAX_TEXT - 1);

    if (msgsnd(queue_id, &msg, sizeof(msg) - sizeof(long), 0) < 0) {
        write_text(STDERR_FILENO, "Failed to notify User 1.\n");
        return 0;
    }

    while (1) {
        if (msgrcv(queue_id, &msg, sizeof(msg) - sizeof(long), USER2_TYPE, 0) < 0) {
            if (errno == EINTR) {
                continue;
            }
            write_text(STDERR_FILENO, "Failed while waiting for chat readiness.\n");
            return 0;
        }

        if (msg.kind == KIND_READY) {
            write_text(STDOUT_FILENO, "User 1 is ready. Chat is ready for both users.\n");
            return 1;
        }
    }
}

/* -------------------- Main -------------------- */

int main(void) {
    int session_fd;
    key_t key;

    signal(SIGINT, cleanup_and_exit);
    signal(SIGUSR1, cleanup_and_exit);

    clear_screen();
    print_header();

    key = ftok(".", 'G');
    if (key == -1) {
        write_text(STDERR_FILENO, "ftok failed.\n");
        return 1;
    }

    session_fd = open(GHOST_FILE, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (session_fd >= 0) {
        is_user1 = 1;
        local_user_id = 1;
        queue_id = msgget(key, IPC_CREAT | 0666);
        close(session_fd);

        if (queue_id < 0) {
            unlink(GHOST_FILE);
            write_text(STDERR_FILENO, "Could not create chat queue.\n");
            return 1;
        }
    } else {
        local_user_id = 2;
        queue_id = msgget(key, 0666);

        if (queue_id < 0) {
            write_text(STDERR_FILENO, "No active chat session found.\n");
            return 1;
        }
    }

    if (!wait_for_ready_state()) {
        cleanup_and_exit(0);
    }

    if (is_user1) {
        timer_pid = fork();
        if (timer_pid == 0) {
            timer_manager_process();
            _exit(0);
        }
    }

    receiver_pid = fork();
    if (receiver_pid == 0) {
        receiver_process();
        _exit(0);
    }

    sender_pid = fork();
    if (sender_pid == 0) {
        sender_process();
        _exit(0);
    }

    waitpid(sender_pid, NULL, 0);
    cleanup_and_exit(0);
    return 0;
}
