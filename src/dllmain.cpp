#define WIN32_LEAN_AND_MEAN             // Selten verwendete Komponenten aus Windows-Headern ausschließen
// Windows-Headerdateien
#include <windows.h>

#include "sorted.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#if 0

#include <iostream>
#include <eventmanager.h>
#include <system_message.h>

using namespace ses;

class hallo_world_message : public system_message {
public:
    hallo_world_message() : system_message(2, 5) {}
    ~hallo_world_message() {
        std::cout << "Bye Bye!\n";
    }
    virtual bool onMessageProcess(void* sender) {
        std::cout << "Hello World!\n";
        return true;
    }
    virtual void onMessagePost(void* sender, bool bWasAdd) {
        std::cout << "onMessagePost" << ((bWasAdd) ? " true " : " false ") << "\n";
    }
    virtual void onMessageExpired(void* sender, uint64_t time) {
        std::cout << "onMessageExpired" << " time: " << time << "\n";
    }
    virtual void onMessageDiscard(void* sender, uint64_t time) {
        std::cout << "onMessageDiscard" << " time: " << time << "\n";
    }
};

int main()
{
    ses::eventmanager manager(300);
    manager.postMessage(std::shared_ptr<hallo_world_message>(new hallo_world_message()), 0);

    if (manager.beginMessages()) {
        manager.processMessages(1, 7);
        manager.endProcessMessages();
    }

   
    return 0;
}
#endif

