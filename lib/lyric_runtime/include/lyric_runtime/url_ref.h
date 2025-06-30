#ifndef LYRIC_RUNTIME_URL_REF_H
#define LYRIC_RUNTIME_URL_REF_H

#include "abstract_ref.h"

namespace lyric_runtime {

    class UrlRef final : public AbstractRef {
    public:
        UrlRef(const ExistentialTable *etable, const LiteralCell &literal);
        UrlRef(const ExistentialTable *etable, const tempo_utils::Url &url);
        ~UrlRef() override;

        const AbstractMemberResolver *getMemberResolver() const override;
        const AbstractMethodResolver *getMethodResolver() const override;
        const AbstractExtensionResolver *getExtensionResolver() const override;

        bool equals(const AbstractRef *other) const override;
        bool rawSize(tu_int32 &size) const override;
        tu_int32 rawCopy(tu_int32 offset, char *dst, tu_int32 size) override;
        bool uriValue(tempo_utils::Url &url) const override;
        bool utf8Value(std::string &utf8) const override;
        bool hashValue(absl::HashState state) override;
        tempo_utils::StatusCode errorStatusCode() override;
        std::string toString() const override;

        lyric_runtime::DataCell uriEquals(UrlRef *other) const;

        tempo_utils::Url getUrl() const;

        bool isReachable() const override;
        void setReachable() override;
        void clearReachable() override;
        void finalize() override;

        /*
         * methods below have the default no-op implementation
         */
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
        lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
        bool iteratorValid() override;
        bool iteratorNext(DataCell &next) override;
        bool prepareFuture(std::shared_ptr<Promise> promise) override;
        bool awaitFuture(SystemScheduler *systemScheduler) override;
        bool resolveFuture(DataCell &result) override;
        bool applyClosure(Task *task, std::vector<DataCell> &args, lyric_runtime::InterpreterState *state) override;

    private:
        const ExistentialTable *m_etable;
        bool m_owned;
        tempo_utils::Url m_url;
        bool m_reachable;
    };
}

#endif // LYRIC_RUNTIME_URL_REF_H
