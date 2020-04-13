#include "encoding.h"
#include <memory>
#include <clocale>
#include <iconv.h>

namespace mirai::utils
{
    namespace
    {
        struct LocaleSetter final
        {
            LocaleSetter()
            {
                std::setlocale(LC_ALL, "");
            }
        } const locale_setter; // Set the locale in static initialization phase

        class IconvDescriptor final
        {
        private:
            void* data_ = nullptr;
        public:
            IconvDescriptor(const char* src, const char* dst) :
                data_(iconv_open(dst, src)) {}
            ~IconvDescriptor() noexcept
            {
                iconv_close(data_);
            }
            std::string convert(const std::string_view text, const float capability_factor) const
            {
                if (text.empty()) return {};
                size_t in_bytes_left = text.size();
                size_t out_bytes_left = size_t(in_bytes_left * capability_factor);
                const std::unique_ptr<char[]> out_buf = std::make_unique<char[]>(out_bytes_left);
                char* in = const_cast<char*>(text.data()); // F*** iconv...
                char* out = out_buf.get();
                if (iconv(data_, &in, &in_bytes_left,
                    &out, &out_bytes_left) == size_t(-1))
                    return {};
                return std::string(out_buf.get());
            }
        };
    }

    std::string convert_encoding(const std::string_view text,
        const char* src_enc, const char* dst_enc, const float capability_factor)
    {
        const IconvDescriptor cd(src_enc, dst_enc);
        return cd.convert(text, capability_factor);
    }

    std::string local_to_utf8(const std::string_view text) { return convert_encoding(text, "", "utf-8"); }
    std::string utf8_to_local(const std::string_view text) { return convert_encoding(text, "utf-8", ""); }
}
