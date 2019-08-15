#include "textconverter.hpp"

#include "log.hpp"
#include "utils.hpp"

#define LOG_MODULE "TextConverter"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

TextConverter::TextConverter() : DataConverter(DATA_FMT_TEXT)
{
}

TextConverter::~TextConverter()
{
}

Status TextConverter::startDecoding(const char* path, void* data, size_t size)
{
    Status status = STS_OK;

    LOG_DEBUG("Start decoding, path: %s, size: %zu", path, size);

    return STS_OK;
}

Status TextConverter::nextDecoding(ResourceData* resourceData)
{
    Status status = STS_OK;

    LOG_DEBUG("Next decoding");

    return STS_OK;
}

Status TextConverter::startEncoding(void* data, size_t size)
{
    return STS_OK;
}

Status TextConverter::nextEncoding(ResourceData* resourceData)
{
    return STS_OK;
}

Status TextConverter::finishEncoding(size_t* size)
{
    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

}  // namespace openlwm2m