/**
    libLaserdockCore
    Copyright(c) 2018 Wicked Lasers

    This file is part of libLaserdockCore.

    libLaserdockCore is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libLaserdockCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libLaserdockCore.  If not, see <https://www.gnu.org/licenses/>.
**/

#ifndef LDSVGFONTMANAGER_H
#define LDSVGFONTMANAGER_H

#include <ldCore/Helpers/Maths/ldMaths.h>
#include <ldCore/Helpers/Text/ldSvgFont.h>

class LDCORESHARED_EXPORT ldSvgFontManager : public QObject
{
    Q_OBJECT
public:
    explicit ldSvgFontManager(QObject *parent = nullptr);
    ~ldSvgFontManager();

    void addFont(const ldSvgFont &font);
    const std::vector<ldSvgFont> &fonts() const;
    const ldSvgFont &font(int index) const;

    QStringList titles(bool excludeCounter = true);

    int counterFontIndex() const;

private:
    std::vector<ldSvgFont> m_fonts;
};


#endif // LDSVGFONTMANAGER_H


