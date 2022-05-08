#include <dlfcn.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <pthread.h>

typedef void (*activate_t)(int, char**);

int main(int argc, char** argv) {
    activate_t activate;
    char* gmodule = "./demo.so";
    void* handle = dlopen(gmodule, RTLD_NOW);
    if (!handle) {
        printf("%s\n", dlerror());
        return 1;
    }
    activate = (activate_t) dlsym(handle, "on_app_activate");
    activate(argc, argv);
    return 0;
}
