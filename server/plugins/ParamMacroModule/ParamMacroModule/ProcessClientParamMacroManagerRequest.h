#ifndef PROCESSCLIENTParamMacroSERVICE_H
#define PROCESSCLIENTParamMacroSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class ProcessClientParamMacroRequestImpl;
class ProcessClientParamMacroRequest : public INetMsg
{
public:
    ProcessClientParamMacroRequest(cppmicroservices::BundleContext context);
    ~ProcessClientParamMacroRequest();

private:
    void AddParamMacro(const ProtocolPack& pack);

    void EditParamMacro(const ProtocolPack& pack);

    void DelParamMacro(const ProtocolPack& pack);

    void GetParamMacro(const ProtocolPack& pack);

    void initData();

    ProcessClientParamMacroRequestImpl* m_impl;
};
#endif  // PROCESSCLIENTSERVICE_H
