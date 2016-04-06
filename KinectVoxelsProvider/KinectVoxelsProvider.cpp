// KinectVoxelsProvider.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "KinectManager.h"
#include "NetworkKinectManager.h"

KinectManager g_kinectManager;

int main()
{
    
    g_kinectManager.Initialize(1.0);

    while (true)
    {
        try
        {
            NetworkKinectManager server;
            server.InititializeServer();
            server.ProcessRequests(&g_kinectManager);
        }
        catch (char* message)
        {
            printf(message);
        }
    }

    return 0;
}

