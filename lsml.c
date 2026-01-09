#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <stddef.h>
#include <stdint.h>
#define Max 1024
#define COLOR_DIR "\033[1;34m"
#define COLOR_LINK "\033[1;36m"
#define COLOR_EXEC "\033[1;32m"
#define COLOR_CHR "\033[1;33m"
#define COLOR_BLK "\033[1;33m"  
#define COLOR_FIFO "\033[1;35m"
#define COLOR_SOCK "\033[1;35m"
#define COLOR_HIDDEN "\033[1;30m"
#define COLOR_ARCHIVE "\033[1;31m"
#define COLOR_DOCUMENT "\033[1;33m"
#define COLOR_IMAGE "\033[1;35m"
#define COLOR_AUDIO "\033[1;36m"
#define COLOR_VIDEO "\033[1;34m"
#define COLOR_CODE "\033[1;32m"
#define COLOR_RESET "\033[0m"
#define MAX_RECURSE_DEPTH 50
#define INITIAL_PATH_SIZE 500
int now[256] = {0};
static int first = 1;
static int recurse_depth = 0;
typedef struct {
    const char* path;
    size_t len;
} SkipDir;
SkipDir skip_dirs[] = 
{
    {"/proc", 5}, {"/sys", 4}, {"/dev", 4}, {"/run", 4},
    {NULL, 0}
};
int wuxiao(const char *dir_path) 
{
    if (strlen(dir_path) == 0) 
        return 1;
    if (recurse_depth > MAX_RECURSE_DEPTH) 
    {
        return 1;
    }
    for (int i = 0; skip_dirs[i].path; i++) 
    {
        size_t skip_len = skip_dirs[i].len;
        if (strncmp(dir_path, skip_dirs[i].path, skip_len) == 0) 
        {
            if (dir_path[skip_len] == '\0' || dir_path[skip_len] == '/') 
            {
                return 1;
            }
        }
    } 
    return 0;
}
void pathjoin(char* dest, size_t dest_size, const char* dir, const char* name) 
{
    if (dest_size == 0 || !name || !dir) 
    {
        if (dest_size > 0) dest[0] = '\0';
        return;
    }
    size_t dir_len = strlen(dir);
    size_t name_len = strlen(name); 
    if (dir_len + name_len + 2 > dest_size) 
    {
        dir_len = dest_size / 2 - 1;
        name_len = dest_size / 2 - 1;
    }
    strncpy(dest, dir, dir_len);
    dest[dir_len] = '\0';
    if (dir_len > 0 && dest[dir_len-1] != '/') 
    {
        if (dir_len + 1 < dest_size) 
        {
            strcat(dest, "/");
            dir_len++;
        }
    }  
    strncat(dest, name, name_len);
    dest[dest_size-1] = '\0';
}
char file_type(const char* pathname) 
{
    struct stat fi;
    if (lstat(pathname, &fi) == -1) 
        return ' ';
    switch (fi.st_mode & S_IFMT) 
    {
        case S_IFREG: return '-';
        case S_IFDIR: return 'd';
        case S_IFCHR: return 'c';
        case S_IFBLK: return 'b';
        case S_IFLNK: return 'l';
        case S_IFIFO: return 'p';
        case S_IFSOCK: return 's';
        default: return '?';
    }
}
typedef struct Path 
{
    char name[256];
    unsigned long node;
    unsigned long blocks;
    time_t mtime; 
    mode_t mode;  
} Path;
int compare_names(const char* a, const char* b) 
{ 
    return strcmp(a, b);
}
int compare_times(const Path* a, const Path* b) 
{
    return a->mtime < b->mtime;
}
void swap_paths(Path* a, Path* b) 
{
    Path temp = *a;
    *a = *b;
    *b = temp;
}
void quick_sort_paths(Path* paths, int left, int right, int use_time) 
{
    if (left >= right) 
    return;
    int pivot = left;
    int i = left + 1;
    int j = right; 
    while (i <= j) 
    {
        int compare_result;
        if (use_time) 
        {
            compare_result = compare_times(&paths[i], &paths[pivot]);
        }
         else
        {
            compare_result = compare_names(paths[i].name, paths[pivot].name) < 0;
        }      
        if ((compare_result != 0) != now['r']) 
        {
            i++;
        }
         else 
        {
            swap_paths(&paths[i], &paths[j]);
            j--;
        }
    }  
    swap_paths(&paths[pivot], &paths[j]);
    quick_sort_paths(paths, left, j - 1, use_time);
    quick_sort_paths(paths, j + 1, right, use_time);
}
void sort_paths(Path* paths, int count) 
{
    if (count <= 1) 
    return;
    if (now['t'] > 0) 
    {
        quick_sort_paths(paths, 0, count - 1, 1);
    }
     else 
     {
        if (count < 50) 
        {
            for (int i = 1; i < count; i++) 
            {
                Path key = paths[i];
                int j = i - 1;
                while (j >= 0 && ((compare_names(paths[j].name, key.name) > 0) != now['r'])) 
                       {
                    paths[j + 1] = paths[j];
                    j--;
                }
                paths[j + 1] = key;
            }
        }
         else
        {
            quick_sort_paths(paths, 0, count - 1, 0);
        }
    }
}
unsigned long calculate_total_blocks(Path* paths, int count) 
{
    unsigned long total = 0;
    for (int i = 0; i < count; i++) 
    {
        total += paths[i].blocks;
    }
    return total / 2;
}
const char* get_file_color_cached(const Path* path, const char* full_path) 
{
    switch (path->mode & S_IFMT) 
    {
        case S_IFDIR: return COLOR_DIR;
        case S_IFLNK: return COLOR_LINK;
        case S_IFCHR: return COLOR_CHR;
        case S_IFBLK: return COLOR_BLK;
        case S_IFIFO: return COLOR_FIFO;
        case S_IFSOCK: return COLOR_SOCK;
        case S_IFREG: 
            if (path->mode & (S_IXUSR|S_IXGRP|S_IXOTH))
                return COLOR_EXEC;
            if (path->name[0] == '.')
                return COLOR_HIDDEN;
            const char* ext = strrchr(path->name, '.');
            if (ext != NULL) {
                ext++;
                if (strcmp(ext, "gz") == 0 || strcmp(ext, "zip") == 0 || strcmp(ext, "tar") == 0 || strcmp(ext, "bz2") == 0 ||strcmp(ext, "xz") == 0 || strcmp(ext, "7z") == 0 || strcmp(ext, "rar") == 0) 
                {
                    return COLOR_ARCHIVE;
                }
                if (strcmp(ext, "pdf") == 0 || strcmp(ext, "doc") == 0 || strcmp(ext, "docx") == 0 || strcmp(ext, "txt") == 0) 
                {
                    return COLOR_DOCUMENT;
                }
                if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0 ||  strcmp(ext, "png") == 0 || strcmp(ext, "gif") == 0) 
                {
                    return COLOR_IMAGE;
                }
                if (strcmp(ext, "mp3") == 0 || strcmp(ext, "wav") == 0) 
                {
                    return COLOR_AUDIO;
                }
                if (strcmp(ext, "mp4") == 0 || strcmp(ext, "avi") == 0) 
                {
                    return COLOR_VIDEO;
                }
                if (strcmp(ext, "c") == 0 || strcmp(ext, "h") == 0 || strcmp(ext, "py") == 0 || strcmp(ext, "sh") == 0) 
                {
                    return COLOR_CODE;
                }
            }
            return COLOR_RESET;
        default: return COLOR_RESET;
    }
}
void print_detailed(const Path* path, const char* base_dir) 
{
    char full_path[Max];
    pathjoin(full_path, Max, base_dir, path->name);
    struct stat st;
    if (lstat(full_path, &st) == -1)
     return;
    printf("%c", file_type(full_path));
    printf("%c%c%c",
        (st.st_mode & S_IRUSR) ? 'r' : '-',
        (st.st_mode & S_IWUSR) ? 'w' : '-',
        (st.st_mode & S_IXUSR) ? ((st.st_mode & S_ISUID) ? 's' : 'x') : ((st.st_mode & S_ISUID) ? 'S' : '-'));
    printf("%c%c%c",
        (st.st_mode & S_IRGRP) ? 'r' : '-',
        (st.st_mode & S_IWGRP) ? 'w' : '-',
        (st.st_mode & S_IXGRP) ? ((st.st_mode & S_ISGID) ? 's' : 'x') : ((st.st_mode & S_ISGID) ? 'S' : '-'));
    printf("%c%c%c ",
        (st.st_mode & S_IROTH) ? 'r' : '-',
        (st.st_mode & S_IWOTH) ? 'w' : '-',
        (st.st_mode & S_IXOTH) ? ((st.st_mode & S_ISVTX) ? 't' : 'x') : ((st.st_mode & S_ISVTX) ? 'T' : '-'));
    printf("%lu ", (unsigned long)st.st_nlink);
    struct passwd* pwd = getpwuid(st.st_uid);
    printf("%-8s ", pwd ? pwd->pw_name : "?");
    struct group* grp = getgrgid(st.st_gid);
    printf("%-8s ", grp ? grp->gr_name : "?");
    printf("%8ld ", (long)st.st_size);
    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", localtime(&st.st_mtime));
    printf("%s ", time_buf);
    const char* color = get_file_color_cached(path, full_path);
    printf("%s%s" COLOR_RESET, color, path->name);
    if ((st.st_mode & S_IFMT) == S_IFLNK) 
    {
        char link_target[Max];
        ssize_t len = readlink(full_path, link_target, Max-1);
        if (len > 0) 
        {
            link_target[len] = '\0';
            printf(" -> %s", link_target);
        }
    }
    printf("\n");
}
Path* allocate_paths(int size) 
{
    Path* p = malloc(sizeof(Path) * size);
    if (!p) 
    {
        perror("malloc");
        exit(1);
    }
    return p;
}
Path* reallocate_paths(Path* p, int new_size) 
{
    Path* np = realloc(p, sizeof(Path) * new_size);
    if (!np) 
    {
        perror("realloc");
        free(p);
        exit(1);
    }
    return np;
}
int get_terminal_width() 
{
    #ifdef TIOCGWINSZ
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) 
        return ws.ws_col;
    #endif
    return 80;
}
int calculate_max_name_length(Path* paths, int count) 
{
    int max = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(paths[i].name);
        if (now['i']) 
        len += 8;
        if (now['s']) 
        len += 5;
        if (len > max) 
        max = len;
    }
    return max;
}
void print_grid(Path* paths, int count, const char* base_dir) 
{
    if (count == 0) return;
    int terminal_width = get_terminal_width();
    int max_len = calculate_max_name_length(paths, count);
    int col_width = (max_len > 8 ? max_len : 8) + 2;
    int cols = terminal_width / col_width;
    if (cols < 1) cols = 1;
    int rows = (count + cols - 1) / cols;
    for (int row = 0; row < rows; row++) 
    {
        for (int col = 0; col < cols; col++) 
        {
            int idx = col * rows + row;
            if (idx >= count) break;
            Path* p = &paths[idx];
            char full_path[Max];
            pathjoin(full_path, Max, base_dir, p->name);
            if (now['i']) 
            {
                struct stat st;
                if (lstat(full_path, &st) == 0) 
                {
                    printf("%7lu ", st.st_ino);
                }
                 else 
                {
                    printf("%7s ", "?");
                }
            }
            if (now['s']) 
            {
                printf("%4lu ", p->blocks / 2);
            }
            int name_width = max_len - (now['i'] ? 8 : 0) - (now['s'] ? 5 : 0);
            const char* color = get_file_color_cached(p, full_path);
            printf("%s%-*s" COLOR_RESET, color, name_width, p->name);
            if (col < cols - 1) 
            printf("  ");
        }
        printf("\n");
    }
}
void list_directory(const char* dir_path, int is_root, int multi_target) 
{
    recurse_depth++;
    if (wuxiao(dir_path)) 
    {
        recurse_depth--;
        return;
    }
    DIR* dir = opendir(dir_path);
    if (!dir) {
        if (errno == EACCES) {
            fprintf(stderr, "'%s'\n", dir_path);
        } else {
            fprintf(stderr, "'%s': %s\n", dir_path, strerror(errno));
        }
        recurse_depth--;
        return;
    }
    Path* paths = allocate_paths(INITIAL_PATH_SIZE);
    int count = 0;
    int capacity = INITIAL_PATH_SIZE;
    struct dirent* entry;
    char full_path[Max];
    while ((entry = readdir(dir)) != NULL) 
    {
        if (!now['a'] && entry->d_name[0] == '.') 
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
            {
                continue;
            }
            if (!now['a']) continue;
        }
        if (count >= capacity) 
        {
            paths = reallocate_paths(paths, capacity * 2);
            capacity *= 2;
        }
        pathjoin(full_path, Max, dir_path, entry->d_name);
        struct stat st;
        if (lstat(full_path, &st) == 0) 
        {
            strncpy(paths[count].name, entry->d_name, 255);
            paths[count].name[255] = '\0';
            paths[count].node = entry->d_ino;
            paths[count].blocks = st.st_blocks;
            paths[count].mtime = st.st_mtime;
            paths[count].mode = st.st_mode;
            count++;
        }
    }
    closedir(dir);
    if (count > 0) 
    {
        if (!now['f']) 
        { 
            sort_paths(paths, count);
        }
        int show_header = (multi_target && is_root) || (now['R'] && recurse_depth > 1) || (!is_root && now['R']);
        if (show_header) 
        {
            if (!first && recurse_depth > 1) 
            printf("\n");
            printf("%s:\n", dir_path);
        }
        first = 0;
        if (now['l']) 
        {
            unsigned long total = calculate_total_blocks(paths, count);
            printf("total %lu\n", total);
            for (int i = 0; i < count; i++) 
            {
                print_detailed(&paths[i], dir_path);
            }
        } 
        else 
        {
            print_grid(paths, count, dir_path);
        }
        if (now['R'] && recurse_depth < MAX_RECURSE_DEPTH) 
        {
            for (int i = 0; i < count; i++) 
            {
                if (strcmp(paths[i].name, ".") == 0 || strcmp(paths[i].name, "..") == 0) 
                {
                    continue;
                }
                if ((paths[i].mode & S_IFMT) == S_IFDIR) 
                {
                    pathjoin(full_path, Max, dir_path, paths[i].name);
                    if ((paths[i].mode & S_IFMT) != S_IFLNK) 
                    {
                        list_directory(full_path, 0, multi_target);
                    }
                }
            }
        }
    } 
    free(paths);
    recurse_depth--;
}
int is_option(const char* arg) 
{
    return arg && arg[0] == '-' && arg[1] != '-' && strlen(arg) >= 2;
}
int main(int argc, char* argv[]) 
{
    int target_count = 0;
    char** targets = NULL;
    int start_idx = 1;
    if (argc > 1 && strcmp(argv[1], "ls") == 0)
    {
        start_idx = 2;
    }
    for (int i = start_idx; i < argc; i++)
    {
        if (is_option(argv[i])) 
        {
            for (int j = 1; argv[i][j]; j++) 
            {
                now[(unsigned char)argv[i][j]] = 1;
            }
        }
    }
    for (int i = start_idx; i < argc; i++) 
    {
        if (!is_option(argv[i])) 
        {
            target_count++;
        }
    }
    if (target_count == 0) 
    {
        target_count = 1;
        targets = malloc(sizeof(char*));
        targets[0] = ".";
    } 
    else 
    {
        targets = malloc(sizeof(char*) * target_count);
        int idx = 0;
        for (int i = start_idx; i < argc; i++) 
        {
            if (!is_option(argv[i])) 
            {
                targets[idx++] = argv[i];
            }
        }
    }
    int exit_code = 0;
    for (int i = 0; i < target_count; i++) 
    {
        const char* target = targets[i];
        struct stat st;
        if (lstat(target, &st) == -1)
        {
            fprintf(stderr, " '%s': %s\n", target, strerror(errno));
            exit_code = 1;
            continue;
        } 
        if (S_ISDIR(st.st_mode)) 
        {
            list_directory(target, (i == 0 && target_count == 1), target_count > 1);
        }
        else 
        {
            Path p;
            strncpy(p.name, target, 255);
            p.name[255] = '\0';
            p.node = st.st_ino;
            p.blocks = st.st_blocks;
            p.mode = st.st_mode;
            p.mtime = st.st_mtime;            
            if (now['l']) 
            {
                print_detailed(&p, "");
            } 
            else 
            {
                const char* color = get_file_color_cached(&p, target);
                printf("%s%s" COLOR_RESET "\n", color, p.name);
            }
        }       
        if (i < target_count - 1) printf("\n");
    }   
    free(targets);
    return exit_code;
}