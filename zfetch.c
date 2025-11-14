/*
 * zFetch - minimalistic system fetcher for Linux platform
 *
 * Copyright (c) 2026 Marco Cetica <email@marcocetica.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 */

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>

#define Z_PATH_MAX 4096
#define IFACE_ENTRY_LEN 64
#define MAX_LOCAL_IPS 8
#define LINE_VALUE_LEN 512

// Colors
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define C1 "\033[31m" // Red
#define C2 "\033[32m" // Green
#define C3 "\033[33m" // Yellow
#define C4 "\033[34m" // Blue
#define C5 "\033[35m" // Magenta
#define C6 "\033[36m" // Cyan
#define C7 "\033[37m" // White
// Normal bars
#define BAR1 "\033[40m   " RESET
#define BAR2 "\033[41m   " RESET
#define BAR3 "\033[42m   " RESET
#define BAR4 "\033[43m   " RESET
#define BAR5 "\033[44m   " RESET
#define BAR6 "\033[45m   " RESET
#define BAR7 "\033[46m   " RESET
#define BAR8 "\033[47m   " RESET
#define BAR9 "\033[100m   " RESET
// Bright bars
#define BAR10 "\033[101m   " RESET
#define BAR11 "\033[102m   " RESET
#define BAR12 "\033[103m   " RESET
#define BAR13 "\033[104m   " RESET
#define BAR14 "\033[105m   " RESET
#define BAR15 "\033[106m   " RESET
#define BAR16 "\033[107m   " RESET
// Bitmasks to enable/disable options
#define OPT_HOST 1U << 0
#define OPT_OS 1U << 1
#define OPT_UPTIME 1U << 2
#define OPT_CPU 1U << 3
#define OPT_MEMORY 1U << 4
#define OPT_DISK 1U << 5
#define OPT_IP 1U << 6
#define OPT_LOGO 1U << 7
#define OPT_BARS 1U << 8

#define ARR_LEN(X) (sizeof(X) / sizeof(X[0]))
#define STRCMP(X, Y) (strcmp(X, Y) == 0)

// Operating system logos
static const char *logo_arch[] = {
    C6 "       /\\",
    C6 "      /  \\",
    C6 "     /\\   \\",
    C4 "    /      \\",
    C4 "   /   ,,   \\",
    C4 "  /   |  |  -\\",
    C4 " /_-''    ''-_\\",
    NULL
};

static const char *logo_artix[] = {
    C6 "      /\\",
    C6 "     /  \\",
    C6 "    /`'.,\\",
    C6 "   /     ',",
    C6 "  /      ,`\\",
    C6 " /   ,.'`.  \\",
    C6 "/.,'`     `'.\\",
    NULL
};

static const char *logo_debian[] = {
    C1 "  _____",
    C1 " /  __ \\",
    C1 "|  /    |",
    C1 "|  \\___-",
    C1 "-_",
    C1 "  --_",
    NULL
};

static const char *logo_fedora[] = {
    C4 "        ,'''''.",
    C4 "       |   ,.  |",
    C4 "       |  |  '_'",
    C4 "  ,....|  |..   ",
    C4 ".'  ,_;|   ..'  ",
    C4 "|  |   |  |     ",
    C4 "|  ',_,'  |     ",
    C4 " '.     ,'      ",
    NULL
};

static const char *logo_gentoo[] = {
    C5 " _-----_",
    C5 "(       \\",
    C5 "\\    0   \\",
    C7 " \\        )",
    C7 " /      _/",
    C7 "(     _-",
    C7 "\\____-",
    NULL
};

static const char *logo_linux[] = {
    C3 "    ___",
    C3 "   (" C7 ".. " C3 "|",
    C3 "   (" C5 "<> " C3 "|",
    C3 "  / " C7 "__  " C3 "\\",
    C3 " ( " C7 "/  \\ " C3 "/|",
    C3 "_/\\ " C7 "__)/" C3 "_)",
    C3 "\\/  " C3 "-____\\/",
    NULL
};

static const char *logo_nixos[] = {
    C4 "  \\\\  \\\\ //",
    C4 " ==\\\\__\\\\/ //",
    C4 "   //   \\\\//",
    C4 "==//     //==",
    C4 " //\\\\___//",
    C4 "// /\\\\  \\\\==",
    C4 "  // \\\\  \\\\",
    NULL
};

static const char *logo_slackware[] = {
    C4 "   ________",
    C4 "  /  ______|",
    C4 "  | |______",
    C4 "  \\______  \\",
    C4 "   ______| |",
    C4 "| |________/",
    C4 "|____________",
    NULL
};

static const char *logo_alpine[] = {
    C4 "   /\\ /\\",
    C4 "  /" C7 "/ " C4 "\\  \\",
    C4 " /" C7 "/   " C4 "\\  \\",
    C4 "/" C7 "//    " C4 "\\  \\",
    C7 "//      " C4 "\\  \\",
    C4 "         \\",
    NULL
};

static const char *logo_ubuntu[] = {
    C3 "       _",
    C3 "   ---(_)",
    C3 " _/  ---  \\",
    C3 "(_) |   |",
    C3 "  \\  --- _/",
    C3 "   ---(_)",
    NULL
};

static const char *logo_mint[] = {
    C2 " __________",
    C2 "|_          \\",
    C2 "  |" C7" | _____" C2 " |",
    C2 "  |" C7" | | | |" C2 " |",
    C2 "  |" C7" | | | |" C2 " |",
    C2 "  |" C7" \\_____/" C2 " |",
    C2 "  \\_________/",
    NULL
};

static const char *logo_suse[] = {
    C2 "  _______",
    C2 "__|   __ \\",
    C2 "     / .\\ \\",
    C2 "     \\__/ |",
    C2 "   _______|",
    C2 "   \\_______",
    C2 "__________/",
    NULL
};

static const char *logo_redhat[] = {
    C1 "      .M.:MMM",
    C1 "     MMMMMMMMMM.",
    C1 "    ,MMMMMMMMMMM",
    C1 " .MM MMMMMMMMMMM",
    C1 "MMMM   MMMMMMMMM",
    C1 "MMMMMM           MM",
    C1 " MMMMMMMMM     ,MMMM",
    C1 "   MMMMMMMMMMMMMMMM:",
    C1 "      `MMMMMMMMMMMM"  ,
    NULL 
};

typedef struct {
    const char *name;
    const char **logo;
} logo_entry_t;

static const logo_entry_t available_logos[] = {
    { C4 "alpine" RESET, logo_alpine },
    { C6 "arch" RESET, logo_arch },
    { C1 "debian" RESET, logo_debian },
    { C4 "fedora" RESET, logo_fedora },
    { C2 "mint" RESET, logo_mint },
    { C5 "gentoo" RESET, logo_gentoo },
    { C6 "artix" RESET, logo_artix },
    { C3 "linux" RESET, logo_linux },
    { C4 "nixos" RESET, logo_nixos },
    { C1 "redhat" RESET, logo_redhat },
    { C4 "slackware" RESET, logo_slackware },
    { C2 "suse" RESET, logo_suse },
    { C3 "ubuntu" RESET, logo_ubuntu }
};

typedef struct {
    const char *label;
    char value[LINE_VALUE_LEN];
} line_t;

typedef struct {
    char id[64];
    char pretty_name[128];
} os_t;

/*
 * Helpers
 */

// Helper function to trim leading and trailing whitespace
static void trim_whitespace(char *str) {
    char *start = str;
    char *end = NULL;
    size_t len = 0;

    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    len = strlen(start);
    if (start != str) {
        memmove(str, start, len + 1);
    }

    if (*str == '\0') { return; }

    end = str + strlen(str) - 1;

    while (end >= str && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }
}

// Helper function to remove leading and trailing quotes
static void strip_quotes(char *str) {
    const size_t len = strlen(str);

    if (len >= 2 && ((str[0] == '"' && str[len - 1] == '"') ||
                    (str[0] == '\'' && str[len - 1] == '\''))) {
        memmove(str, str + 1, len - 2);

        str[len - 2] = '\0';
    }
}

static uint16_t default_opts(void) {
    return OPT_HOST | OPT_OS | OPT_UPTIME | OPT_CPU |
           OPT_MEMORY | OPT_DISK | OPT_IP | OPT_LOGO | OPT_BARS;
}

static void set_option(uint16_t *options, const char *opt, int value) {
    uint16_t flag = 0;

    if (STRCMP(opt, "HOST")) { flag = OPT_HOST; }
    else if (STRCMP(opt, "OS")) { flag = OPT_OS; }
    else if (STRCMP(opt, "UPTIME")) { flag = OPT_UPTIME; }
    else if (STRCMP(opt, "CPU")) { flag = OPT_CPU; }
    else if (STRCMP(opt, "MEMORY")) { flag = OPT_MEMORY; }
    else if (STRCMP(opt, "DISK")) { flag = OPT_DISK; }
    else if (STRCMP(opt, "IP")) { flag = OPT_IP; }
    else if (STRCMP(opt, "LOGO")) { flag = OPT_LOGO; }
    else if (STRCMP(opt, "BARS")) { flag = OPT_BARS; }

    if (flag == 0) { return; }

    if (value) { 
        *options |= flag;
    } else {
        *options &= ~flag;
    }
}

static void parse_config_file(uint16_t *options, const char *path) {
    FILE *fp;
    char line[256];

    fp = fopen(path, "r");
    if (fp == NULL) {
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *eq, *comment, *end;

        // Search comment lines. If found, truncate them
        comment = strchr(line, '#');
        if (comment != NULL) {
            *comment = '\0';
        }

        // Skip empty lines or lines with spaces
        // This also skips truncated lines from the previous step
        trim_whitespace(line);
        if (*line == '\0') { continue; }

        // Look for an equal sign. If not found, skip it
        // Otherwise, truncate at the equal sign
        eq = strchr(line, '=');
        if (eq == NULL) { 
            continue; 
        } else {
            *eq = '\0';
        }

        // The left side becomes the key while the right side becomes the value
        // For example:
        // "CPU = 1" => "CPU_\0_1"
        // key = trim("CPU_"), value = trim("_1")
        trim_whitespace(line); // key
        trim_whitespace(eq + 1); // value

        // Convert value to integer
        errno = 0;
        const long parsed = strtol(eq + 1, &end, 10);

        if (errno != 0 || end == eq + 1 || (*end != '\0' && !isspace((unsigned char)*end))) {
            continue; // Invalid values are skipped
        }

        if ((parsed == 0 || parsed == 1) && *line != '\0') {
            set_option(options, line, (int)parsed);
        }
    }

    fclose(fp);
}

static uint16_t load_config(const char *config_path) {
    char path[Z_PATH_MAX];
    const char *home = getenv("HOME");
    uint16_t options = default_opts();

    if (config_path != NULL) {
        parse_config_file(&options, config_path);
        return options;
    }

    if (home == NULL || *home == '\0') {
        return options;
    }

    snprintf(path, sizeof(path), "%s/.zfetch.conf", home);
    parse_config_file(&options, path);

    // ~/.config takes precedence over $HOME
    snprintf(path, sizeof(path), "%s/.config/zfetch/config", home);
    parse_config_file(&options, path);

    return options;
}

static void read_os_release(os_t *os_rel) {
    FILE *fp = fopen("/etc/os-release", "r");
    char line[256];

    snprintf(os_rel->id, sizeof(os_rel->id), "%s", "linux");
    snprintf(os_rel->pretty_name, sizeof(os_rel->pretty_name), "%s", "linux");

    if (fp == NULL) { return; }

    // The parsing process is the same of the config file parser
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *eq = strchr(line, '=');

        if (eq == NULL) { continue; }

        *eq = '\0';

        trim_whitespace(line); // key
        trim_whitespace(eq + 1); // value
        strip_quotes(eq + 1);

        if (STRCMP(line, "ID")) {
            snprintf(os_rel->id, sizeof(os_rel->id), "%s", eq + 1);
        } else if (STRCMP(line, "PRETTY_NAME")) {
            snprintf(os_rel->pretty_name, sizeof(os_rel->pretty_name), "%s", eq + 1);
        }
    }

    fclose(fp);
}

static const char **get_logo(const char *id) {
    if (id == NULL) { return logo_linux; }
    else if (STRCMP(id, "arch")) { return logo_arch; }
    else if (STRCMP(id, "artix")) { return logo_artix; }
    else if (STRCMP(id, "debian")) { return logo_debian; }
    else if (STRCMP(id, "fedora")) { return logo_fedora; }
    else if (STRCMP(id, "gentoo")) { return logo_gentoo; }
    else if (STRCMP(id, "nixos")) { return logo_nixos; }
    else if (STRCMP(id, "slackware")) { return logo_slackware; }
    else if (STRCMP(id, "alpine")) { return logo_alpine; }
    else if (STRCMP(id, "ubuntu")) { return logo_ubuntu; }
    else if (STRCMP(id, "mint")) { return logo_mint; }
    else if (STRCMP(id, "opensuse") || STRCMP(id, "opensuse-tumbleweed") ||
             STRCMP(id, "opensuse-leap") || STRCMP(id, "sles") || 
             STRCMP(id, "suse")) { return logo_suse; }
    else if (STRCMP(id, "rhel") || STRCMP(id, "redhat") ||
             STRCMP(id, "centos") || STRCMP(id, "rocky") ||
             STRCMP(id, "almalinux")) { return logo_redhat; }

    return logo_linux;
}

static const char *get_percentage_color(double percent) {
    if (percent >= 85.0) { return C1; } // Red
    else if (percent >= 60.0) { return C3; } // Yellow/orange

    return C2; // Green
}

static const char *get_logo_accent(const char **logo) {
    if (logo == logo_arch || logo == logo_artix) {
        return C6; // Cyan
    } else if (logo == logo_debian || logo == logo_redhat) {
        return C1; // Red
    } else if (logo == logo_fedora || logo == logo_nixos ||
               logo == logo_slackware || logo == logo_alpine) {
        return C4; // Blue
    } else if (logo == logo_gentoo) {
        return C5; // Magenta
    } else if (logo == logo_linux || logo == logo_ubuntu) {
        return C3; // Yellow
    } else if (logo == logo_suse || logo == logo_mint) {
        return C2; // Green
    }

    return C3; // Yellow for generic Tux logo
}

// Measure the printable width
static size_t get_printable_length(const char *str) {
    size_t len = 0;

    // Count while ignoring color escape sequences
    while (*str != '\0') {
        if (*str == '\033' && *(str + 1) == '[') {
            str += 2;
            while (*str != '\0' && *str != 'm') { str++; }
            if (*str == 'm') { str++; }
            continue;
        }

        len++;
        str++;
    }

    return len;
}

// Find the widest printable row in order to align the info column nicely
static size_t get_logo_max_width(const char **logo) {
    size_t max = 0;
    size_t idx = 0;

    while (logo[idx] != NULL) {
        size_t width = get_printable_length(logo[idx]);
        if (width > max) {
            max = width;
        }
        idx++;
    }

    return max;
}

static void fmt_size(uint64_t used_kib, uint64_t total_kib, char *buf, size_t buf_size) {
    const char *unit = "MB";
    double divisor = 1024.0;
    double used;
    double total;
    double percent = 0.0;

    if (total_kib >= 1024ULL * 1024ULL) {
        unit = "GB";
        divisor = 1024.0 * 1024.0;
    }

    used = (double)used_kib / divisor;
    total = (double)total_kib / divisor;

    if (total_kib != 0) {
        percent = ((double)used_kib / (double)total_kib) * 100.0;
    }

    snprintf(buf, buf_size, "%.1f %s / %.1f %s (%s%.0f%%%s)",
             used, unit, total, unit, get_percentage_color(percent), percent, RESET);
}

// Retrieve the first line of a file
static bool get_head(const char *path, const char *prefix, char *buf, size_t buf_size) {
    FILE *fp = fopen(path, "r");
    char line[512];

    if (fp == NULL) { return false; }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, prefix, strlen(prefix)) == 0) {
            snprintf(buf, buf_size, "%s", line + strlen(prefix));
            trim_whitespace(buf);
            fclose(fp);

            return true;
        }
    }

    fclose(fp);
    return false;
}

static inline void str_to_lower(char *str) {
    if (str == NULL) { return; }

    while (*str != '\0') {
        *str = (char)tolower((unsigned char)*str);
        str++;
    }
}

static inline size_t fmt_uptime(char *buf, size_t buf_size, size_t offset, uint64_t value, const char *unit) {
    if (value == 0) {
        return offset;
    }

    const char *sep = (offset == 0) ? "" : ", ";

    if (offset >= buf_size) {
        return offset;
    }

    int n = snprintf(buf + offset, buf_size - offset,
                        "%s%" PRIu64 " %s%s",
                        sep, value, unit,
                        value == 1 ? "" : "s");

    if (n < 0) {
        return offset;
    }

    const size_t written = (size_t)n;

    if (written >= buf_size - offset) {
        return buf_size;
    }

    return offset + written;
}

/*
 * Features
 */
static void get_os(char *buf, size_t buf_size) {
    os_t os_release;

    read_os_release(&os_release);
    snprintf(buf, buf_size, "%s", os_release.pretty_name);
}

static void get_hostname(char *buf, size_t buf_size) {
    char hostname[256];

    if (gethostname(hostname, sizeof(hostname)) == 0) {
        hostname[sizeof(hostname) - 1] = '\0';
        snprintf(buf, buf_size, "%s", hostname);
    } else {
        snprintf(buf, buf_size, "unknown");
    }
}

static void get_uptime(char *buf, size_t buf_size) {
    FILE *fp = fopen("/proc/uptime", "r");
    uint64_t total = 0;

    if (fp == NULL) {
        snprintf(buf, buf_size, "unknown");
        return;
    }

    // The /proc/uptime file has two fields formatted as follows:
    // <uptime_in_sec>.<fraction> <idle_in_sec>.<fraction>
    // We only read <uptime_in_sec> and ignore <fraction>
    if (fscanf(fp, "%" SCNu64, &total) != 1) {
        fclose(fp);
        snprintf(buf, buf_size, "unknown");
        return;
    }

    fclose(fp);

    const uint64_t days = total / 86400ULL;
    const uint64_t hours = (total % 86400ULL) / 3600ULL;
    const uint64_t minutes = (total % 3600ULL) / 60ULL;
    const uint64_t seconds = total % 60ULL;

    size_t offset = 0;

    offset = fmt_uptime(buf, buf_size, offset, days, "day");
    offset = fmt_uptime(buf, buf_size, offset, hours, "hour");
    offset = fmt_uptime(buf, buf_size, offset, minutes, "minute");

    if (offset == 0) {
        offset = fmt_uptime(buf, buf_size, offset, seconds, "second");
    }
}

static void get_cpu(char *buf, size_t buf_size) {
    if (!get_head("/proc/cpuinfo", "model name\t: ", buf, buf_size) &&
        !get_head("/proc/cpuinfo", "Hardware\t: ", buf, buf_size)) {
        struct utsname uts;

        if (uname(&uts) == 0) {
            snprintf(buf, buf_size, "%s", uts.machine);
        } else {
            snprintf(buf, buf_size, "unknown");
        }
    }
}

static void get_ram(char *buf, size_t buf_size) {
    FILE *fp = fopen("/proc/meminfo", "r");
    char key[64];
    unsigned long long value;
    unsigned long long total = 0, available = 0;
    bool mem_available = false;

    if (fp == NULL) {
        snprintf(buf, buf_size, "unknown");
        return;
    }

    while (fscanf(fp, "%63s", key) == 1) {
        if (STRCMP(key, "MemTotal:") || STRCMP(key, "MemAvailable:")) {
            if (fscanf(fp, " %llu", &value) != 1) {
                break;
            }

            if (STRCMP(key, "MemTotal:")) {
                total = value;
            } else if (STRCMP(key, "MemAvailable:")) {
                available = value;
                mem_available = true;
            }
        }

        // Consume the rest of the line
        int ch;
        while ((ch = fgetc(fp)) != '\n' && ch != EOF) { }

        if (total != 0 && mem_available) { break; }
    }

    fclose(fp);

    if (total == 0 || !mem_available) {
        snprintf(buf, buf_size, "unknown");
        return;
    }

    fmt_size(total - available, total, buf, buf_size);
}

static void get_disk(char *buf, size_t buf_size) {
    struct statvfs fs;
    uint64_t total_kib, avail_kib, used_kib;

    if (statvfs("/", &fs) != 0) {
        snprintf(buf, buf_size, "unknown");
        return;
    }

    total_kib = ((uint64_t)fs.f_blocks * fs.f_frsize) / 1024ULL;
    avail_kib = ((uint64_t)fs.f_bfree * fs.f_frsize) / 1024ULL;
    used_kib = total_kib - avail_kib;

    fmt_size(used_kib, total_kib, buf, buf_size);
}

static size_t get_ips(char entries[][IFACE_ENTRY_LEN], size_t max_entries) {
    size_t count = 0;
    struct ifaddrs *ifa_list = NULL;

    if (getifaddrs(&ifa_list) != 0) {
        return 0;
    }

    for (struct ifaddrs *ifa = ifa_list; ifa != NULL && count < max_entries; ifa = ifa->ifa_next) {
        char ipv4[INET_ADDRSTRLEN];
        struct sockaddr_in *sin;

        // Skip loopback, disabled interfaces and non-IPv4 interfaces
        if (!ifa->ifa_addr) { continue; }
        if (ifa->ifa_addr->sa_family != AF_INET) { continue; }
        if (!(ifa->ifa_flags & IFF_UP)) { continue; }
        if (!(ifa->ifa_flags & IFF_RUNNING)) { continue; }
        if (ifa->ifa_flags & IFF_LOOPBACK) { continue; }

        sin = (struct sockaddr_in *)ifa->ifa_addr;
        if (!inet_ntop(AF_INET, &sin->sin_addr, ipv4, sizeof(ipv4))) { continue; }

        snprintf(entries[count], IFACE_ENTRY_LEN, "%s (%s)", ipv4, ifa->ifa_name);
        count++;
    }

    freeifaddrs(ifa_list);

    return count;
}

static void get_available_logos(void) {
    const size_t logo_count = ARR_LEN(available_logos);

    if (logo_count == 0) {
        puts("Available logos: none.");
        return;
    }

    printf("Available logos: %s", available_logos[0].name);
    for (size_t idx = 1; idx < logo_count; idx++) {
        printf(", %s", available_logos[idx].name);
    }
    puts(".");
}

static void get_options(uint16_t options, const char *config_path) {
    // Create a temporary mapping type
    struct {
        const char *name;
        uint16_t flag;
    } flags[] = {
        { "HOST", OPT_HOST },
        { "OS", OPT_OS },
        { "UPTIME", OPT_UPTIME },
        { "CPU", OPT_CPU },
        { "MEMORY", OPT_MEMORY },
        { "DISK", OPT_DISK },
        { "IP", OPT_IP },
        { "LOGO", OPT_LOGO },
        { "BARS", OPT_BARS }
    };

    if (config_path) {
        printf(C3 "Using custom config file: '%s'\n" RESET, config_path);
    }

    for (size_t idx = 0; idx < ARR_LEN(flags); idx++) {
        const bool enabled = (options & flags[idx].flag) != 0;

        printf("%-6s : %s%s%s\n", flags[idx].name,
               enabled ? C2 : C1,
               enabled ? "ON" : "OFF",
               RESET);
    }
}

static void system_fetcher(const char **logo, const line_t *lines, size_t line_count, bool show_logo, bool show_bars) {
    const size_t gap = 3;
    const char *accent = get_logo_accent(logo);
    size_t logo_lines = 0, logo_width = 0, total_lines;

    if (show_logo) {
        while (logo[logo_lines] != NULL) { 
            logo_lines++;
        }

        logo_width = get_logo_max_width(logo);
    }

    total_lines = line_count;
    if (show_logo && logo_lines > total_lines) {
        total_lines = logo_lines;
    }

    if (show_bars && line_count + 2 > total_lines) {
        total_lines = line_count + 2;
    }

    for (size_t idx = 0; idx < total_lines; idx++) {
        if (show_logo && idx < logo_lines) {
            printf("%s", logo[idx]);
        }

        if (idx < line_count) {
            if (show_logo && idx < logo_lines) {
                const size_t logo_len = get_printable_length(logo[idx]);
                const size_t padding = (logo_width > logo_len ? logo_width - logo_len : 0) + gap;
                printf("%*s", (int)padding, "");
            } else if (show_logo) {
                printf("%*s", (int)(logo_width + gap), "");
            }
            printf("%s" BOLD "%-10s" RESET " %s", accent, lines[idx].label, lines[idx].value);
        } else if (show_bars && idx == line_count) {
            if (show_logo) {
                if (idx < logo_lines) {
                    const size_t logo_len = get_printable_length(logo[idx]);
                    const size_t padding = (logo_width > logo_len ? logo_width - logo_len : 0) + gap;
                    printf("%*s", (int)padding, "");
                } else {
                    printf("%*s", (int)(logo_width + gap), "");
                }
            }

            printf("%s%s%s%s%s%s%s%s", BAR1, BAR2, BAR3, BAR4,
                   BAR5, BAR6, BAR7, BAR8);
        } else if (show_bars && idx == line_count + 1) {
            if (show_logo) {
                if (idx < logo_lines) {
                    const size_t logo_len = get_printable_length(logo[idx]);
                    const size_t padding = (logo_width > logo_len ? logo_width - logo_len : 0) + gap;
                    printf("%*s", (int)padding, "");
                } else {
                    printf("%*s", (int)(logo_width + gap), "");
                }
            }

            printf("%s%s%s%s%s%s%s%s", BAR9, BAR10, BAR11, BAR12,
                   BAR13, BAR14, BAR15, BAR16);
        }

        putchar('\n');
    }
}

void print_usage(const char *name) {
    printf("%s - minimalistic system fetcher for Linux platform.\n"
           "Available options:\n"
           "-l, --logo       | Manually specify the logo\n"
           "-c, --config     | Specify a configuration file\n"
           "-a, --list-logos | List available logos\n"
           "-s, --list-opts  | Show which options are enable and which are not\n"
           "-h, --help       | Show this helper\n\n"
           "Project homepage: https://github.com/ceticamarco/zfetch\n"
           "                  https://git.marcocetica.com/marco/zfetch\n", name);
}

int main(int argc, char **argv) {
    const struct option long_opts[] = {
        { "logo", required_argument, NULL, 'l' },
        { "config", required_argument, NULL, 'c' },
        { "list-logos", no_argument, NULL, 'a' },
        { "list-opts", no_argument, NULL, 's' },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };
    uint16_t options = 0;
    os_t os_release;
    // number of available options + max number of network interfaces
    line_t lines[9 + IFACE_ENTRY_LEN];
    size_t line_count = 0;
    bool list_logos = false, list_opts = false;
    int opt;
    const char *short_opts = "c:l:ash";
    const char **logo;
    const char *config_path = NULL, *logo_name = NULL;
    char logo_name_norm[64];
    
    opterr = 0; optind = 1;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
        switch (opt) {
        case 'l':
            logo_name = optarg;
            break;
        case 'c':
            config_path = optarg;
            break;
        case 'a':
            list_logos = true;
            break;
        case 's':
            list_opts = true;
            break;
        case 'h':
            print_usage(*(argv + 0));
            return 0;
        default:
            print_usage(*(argv + 0));
            return 1;
        }
    }

    if (optind < argc) {
        print_usage(*(argv + 0));
        return 1;
    }

    if (list_logos) {
        get_available_logos();
        return 0;
    }

    options = load_config(config_path);

    if (list_opts) {
        get_options(options, config_path);
        return 0;
    }

    read_os_release(&os_release);

    if (logo_name != NULL) {
        snprintf(logo_name_norm, sizeof(logo_name_norm), "%s", logo_name);
        str_to_lower(logo_name_norm);
        logo = get_logo(logo_name_norm);
    } else {
        logo = get_logo(os_release.id);
    }

    if (options & OPT_HOST) {
        lines[line_count].label = "Host:";
        get_hostname(lines[line_count].value, sizeof(lines[line_count].value));
        line_count++;
    } 
    
    if (options & OPT_OS) {
        lines[line_count].label = "OS:";
        get_os(lines[line_count].value, sizeof(lines[line_count].value));
        line_count++;
    } 
    
    if (options & OPT_UPTIME) {
        lines[line_count].label = "Uptime:";
        get_uptime(lines[line_count].value, sizeof(lines[line_count].value));
        line_count++;
    }

    if (options & OPT_CPU) {
        lines[line_count].label = "CPU:";
        get_cpu(lines[line_count].value, sizeof(lines[line_count].value));
        line_count++;
    }

    if (options & OPT_MEMORY) {
        lines[line_count].label = "Memory:";
        get_ram(lines[line_count].value, sizeof(lines[line_count].value));
        line_count++;
    }

    if (options & OPT_DISK) {
        lines[line_count].label = "Disk:";
        get_disk(lines[line_count].value, sizeof(lines[line_count].value));
        line_count++;
    }

    if (options & OPT_IP) {
        char interfaces[MAX_LOCAL_IPS][IFACE_ENTRY_LEN];
        const size_t ifaces_count = get_ips(interfaces, MAX_LOCAL_IPS);

        if (ifaces_count == 0) {
            lines[line_count].label = "Local IPs:";
            snprintf(lines[line_count].value, sizeof(lines[line_count].value), "unknown");
            line_count++;
        } else {
            for (size_t idx = 0; idx < ifaces_count && line_count < ARR_LEN(lines); idx++) {
                lines[line_count].label = idx == 0 ? "Local IPs:" : "";
                snprintf(lines[line_count].value, sizeof(lines[line_count].value), "%s%s",
                         interfaces[idx], idx < ifaces_count - 1 ? "," : "");
                line_count++;
            }
        }
    }

    system_fetcher(logo, lines, line_count, (options & OPT_LOGO) != 0, (options & OPT_BARS) != 0);

    return 0;
}
