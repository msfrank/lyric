
#include <openssl/evp.h>

#include <lyric_build/sha256_hash.h>
#include <tempo_utils/log_message.h>

lyric_build::Sha256Hash::Sha256Hash()
{
    m_ctx = EVP_MD_CTX_new();
    TU_ASSERT (m_ctx != nullptr);
    m_dirty = true;
}

lyric_build::Sha256Hash::~Sha256Hash()
{
    if (m_ctx) {
        EVP_MD_CTX_free(static_cast<EVP_MD_CTX *>(m_ctx));
    }
}

bool
lyric_build::Sha256Hash::addData(std::string_view data)
{
    auto *ctx = static_cast<EVP_MD_CTX *>(m_ctx);
    if (m_dirty) {
        if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) == 0)
            return false;
        m_dirty = false;
    }
    if (EVP_DigestUpdate(ctx, data.data(), data.size()) == 0)
        return false;
    return true;
}

std::string
lyric_build::Sha256Hash::getResult()
{
    if (m_dirty)
        return {};

    auto *ctx = static_cast<EVP_MD_CTX *>(m_ctx);
    m_dirty = true;
    std::string result;
    result.resize(EVP_MAX_MD_SIZE);
    unsigned int md_len;
    EVP_DigestFinal_ex(ctx, (unsigned char *) result.data(), &md_len);
    result.resize(md_len);
    return result;
}

std::string
lyric_build::Sha256Hash::hash(std::string_view data)
{
    lyric_build::Sha256Hash hasher;
    hasher.addData(data);
    return hasher.getResult();
}
