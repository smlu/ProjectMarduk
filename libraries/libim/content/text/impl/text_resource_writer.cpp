#include "../text_resource_writer.h"
#include "text_resource_literals.h"

using namespace libim;
using namespace libim::text;
using namespace libim::content::text;
using namespace std::string_view_literals;

TextResourceWriter::TextResourceWriter(OutputStream& os) :
    ostream_(os)
{}

TextResourceWriter& TextResourceWriter::indent(std::size_t width, char indch)
{
    std::string indent(width, indch);
    return write(indent);
}

TextResourceWriter& TextResourceWriter::indent(std::size_t width)
{
    return indent(width, indch_);
}

TextResourceWriter& TextResourceWriter::writeRowIdx(std::size_t idx, std::size_t indent)
{
    const auto strIdx = utils::to_string(idx);
    if(indent > 0)
    {
        const auto strIdxLen =  strIdx.size(); // Note: Do not change this!
                                               //       clang7 and gcc 8 try to optimize out strIdx.size()
                                               //       in std::minmax which then returns garbage
        indent = indent -  std::min(strIdxLen, indent);
        this->indent(indent);
    }

    ostream_ << strIdx << kResLabelPunc;
    return *this;
}

TextResourceWriter& TextResourceWriter::write(std::string_view text)
{
    ostream_ << text;
    return  *this;
}

TextResourceWriter& TextResourceWriter::write(std::string_view text, std::size_t fieldWidth, std::size_t minSep, char indentChar)
{
    write(text);

    auto[min, max] = std::minmax(fieldWidth, std::clamp(text.size(), minSep, fieldWidth));
    auto indw = std::max(minSep, max - min);
    indent(indw, indentChar);
    return *this;
}

TextResourceWriter& TextResourceWriter::writeCommentLine(std::string_view comment)
{
    if(!comment.empty())
    {
        ostream_ << ChComment << ChSpace << comment;
        writeEol();
    }

    return *this;
}

TextResourceWriter& TextResourceWriter::writeEol()
{
    ostream_ << ChEol;
    return *this;
}

TextResourceWriter& TextResourceWriter::writeKeyValue(std::string_view key, std::string_view value, std::size_t indent)
{
    write(key);
    this->indent(indent);
    write(value);
    return writeEol();
}

TextResourceWriter& TextResourceWriter::writeLabel(std::string_view name, std::string_view text)
{
    ostream_ << name << kResLabelPunc << indch_ << text;
    return writeEol();
}

TextResourceWriter& TextResourceWriter::writeLine(std::string_view line)
{
    ostream_ << line;
    return writeEol();
}

TextResourceWriter& TextResourceWriter::writeSection(std::string_view section, bool overline)
{
    if(overline) {
        writeLine(kResSectionHeader);
    }
    return writeLabel(kResName_Section, section);
}