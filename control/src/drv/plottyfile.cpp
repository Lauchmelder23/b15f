#include "plottyfile.h"

void PlottyFile::addDot(Dot& dot)
{
    dots.push_back(dot);
}

void PlottyFile::addDot(Dot dot)
{
    dots.push_back(dot);
}

void PlottyFile::setFunctionType(FunctionType function_type)
{
    this->function_type = function_type;
}

void PlottyFile::setQuadrant(uint8_t quadrant)
{
    if(quadrant < 1 || quadrant > 4)
        throw std::range_error("Ungueltiger Quadrant");
    this->quadrant = quadrant;
}

void PlottyFile::setRefX(uint16_t ref_x)
{
    this->ref_x = ref_x;
}

void PlottyFile::setRefY(uint16_t ref_y)
{
    this->ref_y = ref_y;
}

void PlottyFile::setParaFirstCurve(uint16_t para_first)
{
    this->para_first = para_first;
}

void PlottyFile::setParaStepWidth(uint16_t para_stepwidth)
{
    this->para_stepwidth = para_stepwidth;
}

void PlottyFile::setUnitX(std::string unit_x)
{
    this->unit_x = unit_x;
}

void PlottyFile::setDescX(std::string desc_x)
{
    this->desc_x = desc_x;
}

void PlottyFile::setUnitY(std::string unit_y)
{
    this->unit_y = unit_y;
}

void PlottyFile::setDescY(std::string desc_y)
{
    this->desc_y = desc_y;
}

void PlottyFile::setUnitPara(std::string unit_para)
{
    this->unit_para = unit_para;
}

void PlottyFile::setDescPara(std::string desc_para)
{
    this->desc_para = desc_para;
}

FunctionType PlottyFile::getFunctionType() const
{
    return function_type;
}

uint8_t PlottyFile::getQuadrant() const
{
    return quadrant;
}

uint16_t PlottyFile::getRefX() const
{
    return ref_x;
}

uint16_t PlottyFile::getRefY() const
{
    return ref_y;
}

uint16_t PlottyFile::getParaFirstCurve() const
{
    return para_first;
}

uint16_t PlottyFile::getParaStepWidth() const
{
    return para_stepwidth;
}

std::string PlottyFile::getUnitX() const
{
    return unit_x;
}

std::string PlottyFile::getDescX() const
{
    return desc_x;
}

std::string PlottyFile::getUnitY() const
{
    return unit_y;
}

std::string PlottyFile::getDescY() const
{
    return desc_y;
}

std::string PlottyFile::getUnitPara() const
{
    return unit_para;
}

std::string PlottyFile::getDescPara() const
{
    return desc_para;
}

void PlottyFile::prepStr(std::string& str, uint8_t len)
{
    if(str.length() > len)
        throw std::runtime_error("Zu grosser String.");

    if(str.length() != len)
        str += '\n';

    while(str.length() < len)
        str += '\0';
}

void PlottyFile::writeToFile(std::string filename)
{
    if(dots.empty())
        throw std::length_error("Es wurden keine Punkte gespeichert.");

    prepStr(unit_x, STR_LEN_SHORT);
    prepStr(desc_x, STR_LEN_LARGE);
    prepStr(unit_y, STR_LEN_SHORT);
    prepStr(desc_y, STR_LEN_LARGE);
    prepStr(unit_para, STR_LEN_SHORT);
    prepStr(desc_para, STR_LEN_LARGE);

    std::ofstream file(filename);

    // write file header
    file.write(reinterpret_cast<char*>(&command), 1);
    file.write(head.c_str(), head.length());
    file.write(filetype.c_str(), filetype.length());
    file.write(reinterpret_cast<char*>(&version), 2);
    file.write(reinterpret_cast<char*>(&subversion), 2);
    file.put(static_cast<uint8_t>(function_type));
    file.write(reinterpret_cast<char*>(&quadrant), 1);
    file.write(reinterpret_cast<char*>(&ref_x), 2);
    file.write(reinterpret_cast<char*>(&ref_y), 2);
    file.write(reinterpret_cast<char*>(&para_first), 2);
    file.write(reinterpret_cast<char*>(&para_stepwidth), 2);
    file.write(unit_x.c_str(), unit_x.length());
    file.write(desc_x.c_str(), desc_x.length());
    file.write(unit_y.c_str(), unit_y.length());
    file.write(desc_y.c_str(), desc_y.length());
    file.write(unit_para.c_str(), unit_para.length());
    file.write(desc_para.c_str(), desc_para.length());
    file.write(reinterpret_cast<const char*>(&eof), 1);

    // make sure header size is 256 Byte
    while(file.tellp() < 256)
        file.put(0);

    for(Dot& dot : dots)
    {
        file.put((dot.getX() >> 8) | (static_cast<uint8_t>(dot.getCurve()) << 2));
        file.put(dot.getX() & 0xFF);
        file.put(dot.getY() >> 8);
        file.put(dot.getY() & 0xFF);
    }

    file.close();
}

void PlottyFile::startPlotty(std::string filename)
{
    int code = system(("plotty --in " + filename).c_str());
    if(code)
        throw std::runtime_error("Fehler beim Aufruf von plotty");
}
