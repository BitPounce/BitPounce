#pragma once

#include "Console.h"

namespace BitPounce
{
    // =========================================================
    // VARIABLE REGISTRAR
    // =========================================================
    class VarRegistrar
    {
    public:
        VarRegistrar(const Variable& var)
        {
            Console::AddVariable(var);
        }
    };

    // =========================================================
    // COMMAND REGISTRAR
    // =========================================================
    class CmdRegistrar
    {
    public:
        CmdRegistrar(const Command& cmd)
        {
            Console::AddCommand(cmd);
        }
    };

    #define BP_REGISTER_VAR(varName, ref)                                    \
    static BitPounce::VarRegistrar BP_CONCAT(_bp_var_, __LINE__)(        \
        BitPounce::VariableFactory::Bind(varName, ref))

    #define BP_REGISTER_CMD(cmdName, helpText, lambda)                       \
    static BitPounce::CmdRegistrar BP_CONCAT(_bp_cmd_, __LINE__)(       \
        BitPounce::Command{ cmdName, lambda, helpText })
}