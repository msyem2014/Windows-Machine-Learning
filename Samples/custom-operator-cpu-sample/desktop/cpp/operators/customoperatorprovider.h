#pragma once

#include "noisyrelu_cpu.h"
#include "relu_cpu.h"

struct CustomOperatorProvider :
    winrt::implements<
        CustomOperatorProvider,
        winrt::Windows::AI::MachineLearning::ILearningModelOperatorProvider,
        ILearningModelOperatorProviderNative>
{
    HMODULE m_library;
    winrt::com_ptr<IMLOperatorRegistry> m_registry;

    CustomOperatorProvider()
    {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        m_library = LoadLibraryW(L"windows.ai.machinelearning.dll");
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PC_APP)
        m_library = LoadPackagedLibrary(L"windows.ai.machinelearning.dll", 0 /*Reserved*/);
#endif
        using create_registry_delegate = HRESULT WINAPI (_COM_Outptr_ IMLOperatorRegistry** registry);
        auto create_registry = reinterpret_cast<create_registry_delegate*>(GetProcAddress(m_library, "MLCreateOperatorRegistry"));
        if (FAILED(create_registry(m_registry.put())))
        {
            __fastfail(0);
        }

        RegisterSchemas();
        RegisterKernels();
    }   

    ~CustomOperatorProvider()
    {
        FreeLibrary(m_library);
    }

    void RegisterSchemas()
    {
        NoisyReluOperatorFactory::RegisterNoisyReluSchema(m_registry);
    }

    void RegisterKernels()
    {
        // Replace the Relu operator kernel
        ReluOperatorFactory::RegisterReluKernel(m_registry);

        // Add a new operator kernel for Relu
        NoisyReluOperatorFactory::RegisterNoisyReluKernel(m_registry);
    }

    STDMETHOD(GetRegistry)(IMLOperatorRegistry** ppOperatorRegistry)
    {
        m_registry.copy_to(ppOperatorRegistry);
        return S_OK;
    }
};
