
#include <lyric_archiver/copy_call.h>
#include <lyric_archiver/copy_impl.h>

tempo_utils::Status
lyric_archiver::copy_impl(
    lyric_importer::ImplImport *implImport,
    lyric_assembler::ImplHandle *implHandle,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{

    for (auto it = implImport->extensionsBegin(); it != implImport->extensionsEnd(); it++) {
        const auto &name = it->first;
        const auto &extension = it->second;

        lyric_importer::CallImport *callImport;
        TU_ASSIGN_OR_RETURN (callImport, archiverState.importCall(extension.callUrl));

        TU_RETURN_IF_NOT_OK (define_extension(name, callImport, implHandle, symbolReferenceSet, archiverState));
    }

    return {};
}

tempo_utils::Status
lyric_archiver::define_extension(
    const std::string &name,
    lyric_importer::CallImport *callImport,
    lyric_assembler::ImplHandle *implHandle,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    lyric_assembler::ParameterPack parameterPack;

    for (auto it = callImport->listParametersBegin(); it != callImport->listParametersEnd(); it++) {
        lyric_assembler::Parameter p;
        p.index = it->index;
        p.name = it->name;
        p.typeDef = it->type->getTypeDef();
        p.isVariable = it->isVariable;
        p.placement = it->placement;
        parameterPack.listParameters.push_back(std::move(p));
    }

    for (auto it = callImport->namedParametersBegin(); it != callImport->namedParametersEnd(); it++) {
        lyric_assembler::Parameter p;
        p.index = it->index;
        p.name = it->name;
        p.typeDef = it->type->getTypeDef();
        p.isVariable = it->isVariable;
        p.placement = it->placement;
        parameterPack.namedParameters.push_back(std::move(p));
    }

    if (callImport->hasRestParameter()) {
        auto rest = callImport->getRestParameter();
        lyric_assembler::Parameter p;
        p.index = rest.index;
        p.name = rest.name;
        p.typeDef = rest.type->getTypeDef();
        p.isVariable = rest.isVariable;
        p.placement = rest.placement;
        parameterPack.restParameter = Option(p);
    }

    auto returnType = callImport->getReturnType()->getTypeDef();

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, implHandle->defineExtension(name, parameterPack, returnType));

    return put_pending_proc(callImport, procHandle, symbolReferenceSet, archiverState);
}
