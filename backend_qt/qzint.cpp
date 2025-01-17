/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bogdan@licentia.eu                                                    *
 *   Copyright (C) 2010-2021 Robin Stuart                                  *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/
/* vim: set ts=4 sw=4 et : */

#ifdef _MSC_VER
#if _MSC_VER >= 1900 /* MSVC 2015 */
#pragma warning(disable: 4996) /* function or variable may be unsafe */
#endif
#endif

//#include <QDebug>
#include "qzint.h"
#include <stdio.h>
#include <math.h>
#include <QFontMetrics>
/* the following include was necessary to compile with Qt 5.15 on Windows */
/* Qt 5.7 did not require it. */
#include <QPainterPath>

// Shorthand
#define QSL QStringLiteral

namespace Zint {
    static const char *fontStyle = "Helvetica";
    static const char *fontStyleError = "Helvetica";
    static const int fontSizeError = 14; /* Point size */

    QZint::QZint()
        : m_zintSymbol(NULL), m_symbol(BARCODE_CODE128), m_input_mode(UNICODE_MODE),
            m_height(0.0f),
            m_option_1(-1), m_option_2(0), m_option_3(0),
            m_scale(1.0f),
            m_dotty(false), m_dot_size(4.0f / 5.0f),
            m_guardDescent(5.0f),
            m_fgColor(Qt::black), m_bgColor(Qt::white), m_cmyk(false),
            m_borderType(0), m_borderWidth(0),
            m_whitespace(0), m_vwhitespace(0),
            m_fontSetting(0),
            m_show_hrt(true),
            m_gssep(false),
            m_quiet_zones(false), m_no_quiet_zones(false),
            m_compliant_height(false),
            m_rotate_angle(0),
            m_eci(0),
            m_gs1parens(false), m_gs1nocheck(false),
            m_reader_init(false),
            m_warn_level(WARN_DEFAULT), m_debug(false),
            m_encodedWidth(0), m_encodedRows(0),
            m_error(0),
            target_size_horiz(0), target_size_vert(0) // Legacy
    {
        memset(&m_structapp, 0, sizeof(m_structapp));
    }

    QZint::~QZint() {
        if (m_zintSymbol)
            ZBarcode_Delete(m_zintSymbol);
    }

    void QZint::resetSymbol() {
        if (m_zintSymbol)
            ZBarcode_Delete(m_zintSymbol);

        m_lastError.clear();
        m_zintSymbol = ZBarcode_Create();
        m_zintSymbol->output_options |= m_borderType | m_fontSetting;
        m_zintSymbol->symbology = m_symbol;
        m_zintSymbol->height = m_height;
        m_zintSymbol->whitespace_width = m_whitespace;
        m_zintSymbol->whitespace_height = m_vwhitespace;
        if (m_quiet_zones) {
            m_zintSymbol->output_options |= BARCODE_QUIET_ZONES;
        }
        if (m_no_quiet_zones) {
            m_zintSymbol->output_options |= BARCODE_NO_QUIET_ZONES;
        }
        if (m_compliant_height) {
            m_zintSymbol->output_options |= COMPLIANT_HEIGHT;
        }
        m_zintSymbol->border_width = m_borderWidth;
        m_zintSymbol->option_1 = m_option_1;
        m_zintSymbol->option_2 = m_option_2;
        m_zintSymbol->option_3 = m_option_3;
        m_zintSymbol->input_mode = m_input_mode;
        if (m_dotty) {
            m_zintSymbol->output_options |= BARCODE_DOTTY_MODE;
        }
        m_zintSymbol->dot_size = m_dot_size;
        m_zintSymbol->guard_descent = m_guardDescent;
        m_zintSymbol->structapp = m_structapp;
        m_zintSymbol->show_hrt = m_show_hrt ? 1 : 0;
        m_zintSymbol->eci = m_eci;
        m_zintSymbol->scale = m_scale;
        if (m_gs1parens) {
            m_zintSymbol->input_mode |= GS1PARENS_MODE;
        }
        if (m_gs1nocheck) {
            m_zintSymbol->input_mode |= GS1NOCHECK_MODE;
        }
        if (m_gssep) {
            m_zintSymbol->output_options |= GS1_GS_SEPARATOR;
        }
        if (m_reader_init) {
            m_zintSymbol->output_options |= READER_INIT;
        }
        if (m_warn_level) {
            m_zintSymbol->warn_level = m_warn_level;
        }
        if (m_debug) {
            m_zintSymbol->debug |= ZINT_DEBUG_PRINT;
        }

        strcpy(m_zintSymbol->fgcolour, m_fgColor.name().toLatin1().right(6));
        if (m_fgColor.alpha() != 0xFF) {
            strcat(m_zintSymbol->fgcolour, m_fgColor.name(QColor::HexArgb).toLatin1().mid(1,2));
        }
        strcpy(m_zintSymbol->bgcolour, m_bgColor.name().toLatin1().right(6));
        if (m_bgColor.alpha() != 0xFF) {
            strcat(m_zintSymbol->bgcolour, m_bgColor.name(QColor::HexArgb).toLatin1().mid(1,2));
        }
        if (m_cmyk) {
            m_zintSymbol->output_options |= CMYK_COLOUR;
        }
        strcpy(m_zintSymbol->primary, m_primaryMessage.toLatin1().left(127));
    }

    void QZint::encode() {
        resetSymbol();
        QByteArray bstr = m_text.toUtf8();
        /* Note do our own rotation */
        m_error = ZBarcode_Encode_and_Buffer_Vector(m_zintSymbol, (unsigned char *) bstr.data(), bstr.length(), 0);
        m_lastError = m_zintSymbol->errtxt;

        if (m_error < ZINT_ERROR) {
            m_borderType = m_zintSymbol->output_options & (BARCODE_BIND | BARCODE_BOX);
            m_height = m_zintSymbol->height;
            m_borderWidth = m_zintSymbol->border_width;
            m_whitespace = m_zintSymbol->whitespace_width;
            m_vwhitespace = m_zintSymbol->whitespace_height;
            m_encodedWidth = m_zintSymbol->width;
            m_encodedRows = m_zintSymbol->rows;
            emit encoded();
        } else {
            m_encodedWidth = 0;
            m_encodedRows = 0;
            emit errored();
        }
    }

    int QZint::symbol() const {
        return m_symbol;
    }

    void QZint::setSymbol(int symbol) {
        m_symbol = symbol;
    }

    int QZint::inputMode() const {
        return m_input_mode;
    }

    void QZint::setInputMode(int input_mode) {
        m_input_mode = input_mode;
    }

    QString QZint::text() const {
        return m_text;
    }

    void QZint::setText(const QString& text) {
        m_text = text;
    }

    QString QZint::primaryMessage() const {
        return m_primaryMessage;
    }

    void QZint::setPrimaryMessage(const QString& primaryMessage) {
        m_primaryMessage = primaryMessage;
    }

    float QZint::height() const {
        return m_height;
    }

    void QZint::setHeight(float height) {
        m_height = height;
    }

    int QZint::option1() const {
        return m_option_1;
    }

    void QZint::setOption1(int option_1) {
        m_option_1 = option_1;
    }

    int QZint::option2() const {
        return m_option_2;
    }

    void QZint::setOption2(int option) {
        m_option_2 = option;
    }

    int QZint::option3() const {
        return m_option_3;
    }

    void QZint::setOption3(int option) {
        m_option_3 = option;
    }

    float QZint::scale() const {
        return m_scale;
    }

    void QZint::setScale(float scale) {
        m_scale = scale;
    }

    bool QZint::dotty() const {
        return m_dotty;
    }

    void QZint::setDotty(bool dotty) {
        m_dotty = dotty;
    }

    float QZint::dotSize() const {
        return m_dot_size;
    }

    void QZint::setDotSize(float dotSize) {
        m_dot_size = dotSize;
    }

    float QZint::guardDescent() const {
        return m_guardDescent;
    }

    void QZint::setGuardDescent(float guardDescent) {
        m_guardDescent = guardDescent;
    }

    int QZint::structAppCount() const {
        return m_structapp.count;
    }

    int QZint::structAppIndex() const {
        return m_structapp.index;
    }

    QString QZint::structAppID() const {
        return m_structapp.id;
    }

    void QZint::setStructApp(const int count, const int index, const QString& id) {
        if (count) {
            m_structapp.count = count;
            m_structapp.index = index;
            memset(m_structapp.id, 0, sizeof(m_structapp.id));
            if (!id.isEmpty()) {
                QByteArray idArr = id.toLatin1();
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
                strncpy(m_structapp.id, idArr, sizeof(m_structapp.id));
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
            }
        } else {
            clearStructApp();
        }
    }

    void QZint::clearStructApp() {
        memset(&m_structapp, 0, sizeof(m_structapp));
    }

    QColor QZint::fgColor() const {
        return m_fgColor;
    }

    void QZint::setFgColor(const QColor& fgColor) {
        m_fgColor = fgColor;
    }

    QColor QZint::bgColor() const {
        return m_bgColor;
    }

    void QZint::setBgColor(const QColor& bgColor) {
        m_bgColor = bgColor;
    }

    bool QZint::cmyk() const {
        return m_cmyk;
    }

    void QZint::setCMYK(bool cmyk) {
        m_cmyk = cmyk;
    }

    int QZint::borderType() const {
        return m_borderType;
    }

    void QZint::setBorderType(int borderTypeIndex) {
        if (borderTypeIndex == 1) {
            m_borderType = BARCODE_BIND;
        } else if (borderTypeIndex == 2) {
            m_borderType = BARCODE_BOX;
        } else {
            m_borderType = 0;
        }
    }

    int QZint::borderWidth() const {
        return m_borderWidth;
    }

    void QZint::setBorderWidth(int borderWidth) {
        if (borderWidth < 0 || borderWidth > 16)
            borderWidth = 0;
        m_borderWidth = borderWidth;
    }

    int QZint::whitespace() const {
        return m_whitespace;
    }

    void QZint::setWhitespace(int whitespace) {
        m_whitespace = whitespace;
    }

    int QZint::vWhitespace() const {
        return m_vwhitespace;
    }

    void QZint::setVWhitespace(int vWhitespace) {
        m_vwhitespace = vWhitespace;
    }

    int QZint::fontSetting() const {
        return m_fontSetting;
    }

    void QZint::setFontSetting(int fontSettingIndex) { // Sets from comboBox index
        if (fontSettingIndex == 1) {
            m_fontSetting = BOLD_TEXT;
        } else if (fontSettingIndex == 2) {
            m_fontSetting = SMALL_TEXT;
        } else if (fontSettingIndex == 3) {
            m_fontSetting = SMALL_TEXT | BOLD_TEXT;
        } else {
            m_fontSetting = 0;
        }
    }

    void QZint::setFontSettingValue(int fontSetting) { // Sets literal value
        if ((fontSetting & (BOLD_TEXT | SMALL_TEXT)) == fontSetting) {
            m_fontSetting = fontSetting;
        } else {
            m_fontSetting = 0;
        }
    }

    bool QZint::showText() const {
        return m_show_hrt;
    }

    void QZint::setShowText(bool showText) {
        m_show_hrt = showText;
    }

    bool QZint::gsSep() const {
        return m_gssep;
    }

    void QZint::setGSSep(bool gsSep) {
        m_gssep = gsSep;
    }

    bool QZint::quietZones() const {
        return m_quiet_zones;
    }

    void QZint::setQuietZones(bool quietZones) {
        m_quiet_zones = quietZones;
    }

    bool QZint::noQuietZones() const {
        return m_no_quiet_zones;
    }

    void QZint::setNoQuietZones(bool noQuietZones) {
        m_no_quiet_zones = noQuietZones;
    }

    bool QZint::compliantHeight() const {
        return m_compliant_height;
    }

    void QZint::setCompliantHeight(bool compliantHeight) {
        m_compliant_height = compliantHeight;
    }

    int QZint::rotateAngle() const {
        return m_rotate_angle;
    }

    void QZint::setRotateAngle(int rotateIndex) { // Sets from comboBox index
        if (rotateIndex == 1) {
            m_rotate_angle = 90;
        } else if (rotateIndex == 2) {
            m_rotate_angle = 180;
        } else if (rotateIndex == 3) {
            m_rotate_angle = 270;
        } else {
            m_rotate_angle = 0;
        }
    }

    void QZint::setRotateAngleValue(int rotateAngle) { // Sets literal value
        if (rotateAngle == 90) {
            m_rotate_angle = 90;
        } else if (rotateAngle == 180) {
            m_rotate_angle = 180;
        } else if (rotateAngle == 270) {
            m_rotate_angle = 270;
        } else {
            m_rotate_angle = 0;
        }
    }

    int QZint::eci() const {
        return m_eci;
    }

    void QZint::setECI(int ECIIndex) { // Sets from comboBox index
        if (ECIIndex >= 1 && ECIIndex <= 11) {
            m_eci = ECIIndex + 2;
        } else if (ECIIndex >= 12 && ECIIndex <= 15) {
            m_eci = ECIIndex + 3;
        } else if (ECIIndex >= 16 && ECIIndex <= 26) {
            m_eci = ECIIndex + 4;
        } else if (ECIIndex == 27) {
            m_eci = 899; /* 8-bit binary data */
        } else {
            m_eci = 0;
        }
    }

    void QZint::setECIValue(int eci) { // Sets literal value
        if (eci < 3 || (eci > 30 && eci != 899) || eci == 14 || eci == 19) {
            m_eci = 0;
        } else {
            m_eci = eci;
        }
    }

    bool QZint::gs1Parens() const {
        return m_gs1parens;
    }

    void QZint::setGS1Parens(bool gs1Parens) {
        m_gs1parens = gs1Parens;
    }

    bool QZint::gs1NoCheck() const {
        return m_gs1nocheck;
    }

    void QZint::setGS1NoCheck(bool gs1NoCheck) {
        m_gs1nocheck = gs1NoCheck;
    }

    bool QZint::readerInit() const {
        return m_reader_init;
    }

    void QZint::setReaderInit(bool readerInit) {
        m_reader_init = readerInit;
    }

    int QZint::warnLevel() const {
        return m_warn_level;
    }

    void QZint::setWarnLevel(int warnLevel) {
        m_warn_level = warnLevel;
    }

    bool QZint::debug() const {
        return m_debug;
    }

    void QZint::setDebug(bool debug) {
        m_debug = debug;
    }

    int QZint::encodedWidth() const { // Read-only, encoded width (no. of modules encoded)
        return m_encodedWidth;
    }

    int QZint::encodedRows() const { // Read-only, no. of rows encoded
        return m_encodedRows;
    }

    /* Legacy */
    void QZint::setWidth(int width) { setOption1(width); }
    int QZint::width() const { return m_option_1; }
    void QZint::setSecurityLevel(int securityLevel) { setOption2(securityLevel); }
    int QZint::securityLevel() const { return m_option_2; }
    void QZint::setPdf417CodeWords(int /*pdf417CodeWords*/) {}
    int QZint::pdf417CodeWords() const { return 0; }
    void QZint::setHideText(bool hide) { setShowText(!hide); }
    void QZint::setTargetSize(int width, int height) {
        target_size_horiz = width;
        target_size_vert = height;
    }
    QString QZint::error_message() const { return m_lastError; } /* Same as lastError() */

    bool QZint::hasHRT(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_HRT);
    }

    bool QZint::isStackable(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_STACKABLE);
    }

    bool QZint::isExtendable(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_EXTENDABLE);
    }

    bool QZint::isComposite(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_COMPOSITE);
    }

    bool QZint::supportsECI(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_ECI);
    }

    bool QZint::supportsGS1(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_GS1);
    }

    bool QZint::isDotty(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_DOTTY);
    }

    bool QZint::hasDefaultQuietZones(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_QUIET_ZONES);
    }

    bool QZint::isFixedRatio(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_FIXED_RATIO);
    }

    bool QZint::supportsReaderInit(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_READER_INIT);
    }

    bool QZint::supportsFullMultibyte(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_FULL_MULTIBYTE);
    }

    bool QZint::hasMask(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_MASK);
    }

    bool QZint::supportsStructApp(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_STRUCTAPP);
    }

    bool QZint::hasCompliantHeight(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_COMPLIANT_HEIGHT);
    }

    int QZint::getError() const {
        return m_error;
    }

    const QString& QZint::lastError() const {
        return m_lastError;
    }

    bool QZint::hasErrors() const {
        return m_lastError.length();
    }

    int QZint::getVersion() const {
        return ZBarcode_Version();
    }

    bool QZint::save_to_file(const QString &filename) {
        resetSymbol();
        strcpy(m_zintSymbol->outfile, filename.toLatin1().left(255));
        QByteArray bstr = m_text.toUtf8();
        m_error = ZBarcode_Encode_and_Print(m_zintSymbol, (unsigned char *) bstr.data(), bstr.length(),
                                            m_rotate_angle);
        if (m_error >= ZINT_ERROR) {
            m_lastError = m_zintSymbol->errtxt;
            m_encodedWidth = 0;
            m_encodedRows = 0;
            emit errored();
            return false;
        } else {
            return true;
        }
    }

    Qt::GlobalColor QZint::colourToQtColor(int colour) {
        switch (colour) {
            case 1: // Cyan
                return Qt::cyan;
                break;
            case 2: // Blue
                return Qt::blue;
                break;
            case 3: // Magenta
                return Qt::magenta;
                break;
            case 4: // Red
                return Qt::red;
                break;
            case 5: // Yellow
                return Qt::yellow;
                break;
            case 6: // Green
                return Qt::green;
                break;
            case 8: // White
                return Qt::white;
                break;
            default:
                return Qt::black;
                break;
        }
    }

    /* Note: legacy argument `mode` is not used */
    void QZint::render(QPainter& painter, const QRectF& paintRect, AspectRatioMode /*mode*/) {
        struct zint_vector_rect *rect;
        struct zint_vector_hexagon *hex;
        struct zint_vector_circle *circle;
        struct zint_vector_string *string;

        encode();

        painter.save();

        if (m_error >= ZINT_ERROR) {
            painter.setRenderHint(QPainter::Antialiasing);
            QFont font(fontStyleError, fontSizeError);
            painter.setFont(font);
            painter.drawText(paintRect, Qt::AlignCenter | Qt::TextWordWrap, m_lastError);
            painter.restore();
            return;
        }

        painter.setClipRect(paintRect, Qt::IntersectClip);

        qreal xtr = paintRect.x();
        qreal ytr = paintRect.y();
        qreal scale;

        qreal gwidth = m_zintSymbol->vector->width;
        qreal gheight = m_zintSymbol->vector->height;

        if (m_rotate_angle == 90 || m_rotate_angle == 270) {
            if (paintRect.width() / gheight < paintRect.height() / gwidth) {
                scale = paintRect.width() / gheight;
            } else {
                scale = paintRect.height() / gwidth;
            }
        } else {
            if (paintRect.width() / gwidth < paintRect.height() / gheight) {
                scale = paintRect.width() / gwidth;
            } else {
                scale = paintRect.height() / gheight;
            }
        }

        xtr += (qreal) (paintRect.width() - gwidth * scale) / 2.0;
        ytr += (qreal) (paintRect.height() - gheight * scale) / 2.0;

        if (m_rotate_angle) {
            painter.translate(paintRect.width() / 2.0, paintRect.height() / 2.0); // Need to rotate around centre
            painter.rotate(m_rotate_angle);
            painter.translate(-paintRect.width() / 2.0, -paintRect.height() / 2.0); // Undo
        }

        painter.translate(xtr, ytr);
        painter.scale(scale, scale);

        QBrush bgBrush(m_bgColor);
        painter.fillRect(QRectF(0, 0, gwidth, gheight), bgBrush);

        // Plot rectangles
        rect = m_zintSymbol->vector->rectangles;
        if (rect) {
            QBrush brush(Qt::SolidPattern);
            while (rect) {
                if (rect->colour == -1) {
                    brush.setColor(m_fgColor);
                } else {
                    brush.setColor(colourToQtColor(rect->colour));
                }
                painter.fillRect(QRectF(rect->x, rect->y, rect->width, rect->height), brush);
                rect = rect->next;
            }
        }

        // Plot hexagons
        hex = m_zintSymbol->vector->hexagons;
        if (hex) {
            painter.setRenderHint(QPainter::Antialiasing);
            QBrush fgBrush(m_fgColor);
            qreal previous_diameter = 0.0, radius = 0.0, half_radius = 0.0, half_sqrt3_radius = 0.0;
            while (hex) {
                if (previous_diameter != hex->diameter) {
                    previous_diameter = hex->diameter;
                    radius = 0.5 * previous_diameter;
                    half_radius = 0.25 * previous_diameter;
                    half_sqrt3_radius = 0.43301270189221932338 * previous_diameter;
                }

                QPainterPath pt;
                pt.moveTo(hex->x, hex->y + radius);
                pt.lineTo(hex->x + half_sqrt3_radius, hex->y + half_radius);
                pt.lineTo(hex->x + half_sqrt3_radius, hex->y - half_radius);
                pt.lineTo(hex->x, hex->y - radius);
                pt.lineTo(hex->x - half_sqrt3_radius, hex->y - half_radius);
                pt.lineTo(hex->x - half_sqrt3_radius, hex->y + half_radius);
                pt.lineTo(hex->x, hex->y + radius);
                painter.fillPath(pt, fgBrush);

                hex = hex->next;
            }
        }

        // Plot dots (circles)
        circle = m_zintSymbol->vector->circles;
        if (circle) {
            painter.setRenderHint(QPainter::Antialiasing);
            QPen p;
            QBrush fgBrush(m_fgColor);
            qreal previous_diameter = 0.0, radius = 0.0;
            while (circle) {
                if (previous_diameter != circle->diameter) {
                    previous_diameter = circle->diameter;
                    radius = 0.5 * previous_diameter;
                }
                if (circle->colour) { // Set means use background colour
                    p.setColor(m_bgColor);
                    p.setWidthF(circle->width);
                    painter.setPen(p);
                    painter.setBrush(circle->width ? Qt::NoBrush : bgBrush);
                } else {
                    p.setColor(m_fgColor);
                    p.setWidthF(circle->width);
                    painter.setPen(p);
                    painter.setBrush(circle->width ? Qt::NoBrush : fgBrush);
                }
                painter.drawEllipse(QPointF(circle->x, circle->y), radius, radius);
                circle = circle->next;
            }
        }

        // Plot text
        string = m_zintSymbol->vector->strings;
        if (string) {
            painter.setRenderHint(QPainter::Antialiasing);
            QPen p;
            p.setColor(m_fgColor);
            painter.setPen(p);
            bool bold = (m_zintSymbol->output_options & BOLD_TEXT)
                            && (!isExtendable() || (m_zintSymbol->output_options & SMALL_TEXT));
            QFont font(fontStyle, -1 /*pointSize*/, bold ? QFont::Bold : -1);
            while (string) {
                font.setPixelSize(string->fsize);
                painter.setFont(font);
                QString content = QString::fromUtf8((const char *) string->text);
                /* string->y is baseline of font */
                if (string->halign == 1) { /* Left align */
                    painter.drawText(QPointF(string->x, string->y), content);
                } else {
                    QFontMetrics fm(painter.fontMetrics());
                    int width = fm.boundingRect(content).width();
                    if (string->halign == 2) { /* Right align */
                        painter.drawText(QPointF(string->x - width, string->y), content);
                    } else { /* Centre align */
                        painter.drawText(QPointF(string->x - (width / 2.0), string->y), content);
                    }
                }
                string = string->next;
            }
        }

        painter.restore();
    }

    /* Translate settings into Command Line equivalent. Set `win` to use Windows escaping of data.
       If `autoHeight` set then `--height=` option will not be emitted.
       If HEIGHTPERROW_MODE set and non-zero `heightPerRow` given then use that for height instead of internal
       height */
    QString QZint::getAsCLI(const bool win, const bool longOptOnly, const bool barcodeNames,
                    const bool autoHeight, const float heightPerRow, const QString &outfile) const {
        QString cmd(win ? QSL("zint.exe") : QSL("zint"));

        char name_buf[32];
        if (barcodeNames && ZBarcode_BarcodeName(m_symbol, name_buf) == 0) {
            QString name(name_buf + 8); // Strip "BARCODE_" prefix
            arg_str(cmd, longOptOnly ? "--barcode=" : "-b ", name);
        } else {
            arg_int(cmd, longOptOnly ? "--barcode=" : "-b ", m_symbol);
        }

        if (isExtendable()) {
            arg_int(cmd, "--addongap=", option2());
        }

        if (bgColor() != Qt::white && bgColor() != QColor(0xFF, 0xFF, 0xFF, 0)) {
            arg_color(cmd, "--bg=", bgColor());
        }

        arg_bool(cmd, "--binary", (inputMode() & 0x07) == DATA_MODE);
        arg_bool(cmd, "--bind", (borderType() & BARCODE_BIND) && !(borderType() & BARCODE_BOX));
        arg_bool(cmd, "--bold", fontSetting() & BOLD_TEXT);
        arg_int(cmd, "--border=", borderWidth());
        arg_bool(cmd, "--box", borderType() & BARCODE_BOX);
        arg_bool(cmd, "--cmyk", cmyk());

        if (m_symbol == BARCODE_DBAR_EXPSTK || m_symbol == BARCODE_DBAR_EXPSTK_CC
                || m_symbol == BARCODE_PDF417 || m_symbol == BARCODE_PDF417COMP || m_symbol == BARCODE_HIBC_PDF
                || m_symbol == BARCODE_MICROPDF417 || m_symbol == BARCODE_HIBC_MICPDF
                || m_symbol == BARCODE_DOTCODE || m_symbol == BARCODE_CODABLOCKF || m_symbol == BARCODE_HIBC_BLOCKF) {
            arg_int(cmd, "--cols=", option2());
        }

        arg_bool(cmd, "--compliantheight", hasCompliantHeight() && compliantHeight());

        arg_data(cmd, longOptOnly ? "--data=" : "-d ", text(), win);

        if (m_symbol == BARCODE_DATAMATRIX || m_symbol == BARCODE_HIBC_DM) {
            arg_bool(cmd, "--dmre", option3() == DM_DMRE);
        }

        if ((m_symbol == BARCODE_DOTCODE || (isDotty() && dotty())) && dotSize() != 0.8f) {
            arg_float(cmd, "--dotsize=", dotSize());
        }
        if (m_symbol != BARCODE_DOTCODE && isDotty() && dotty()) {
            arg_bool(cmd, "--dotty", dotty());
        }

        if (supportsECI()) {
            arg_int(cmd, "--eci=", eci());
        }

        arg_bool(cmd, "--esc", inputMode() & ESCAPE_MODE);
        arg_bool(cmd, "--fast", inputMode() & FAST_MODE);

        if (fgColor() != Qt::black) {
            arg_color(cmd, "--fg=", fgColor());
        }

        arg_bool(cmd, "--fullmultibyte", supportsFullMultibyte() && (option3() & 0xFF) == ZINT_FULL_MULTIBYTE);

        if (supportsGS1()) {
            arg_bool(cmd, "--gs1", (inputMode() & 0x07) == GS1_MODE);
            arg_bool(cmd, "--gs1parens", gs1Parens() || (inputMode() & GS1PARENS_MODE));
            arg_bool(cmd, "--gs1nocheck", gs1NoCheck() || (inputMode() & GS1NOCHECK_MODE));
            arg_bool(cmd, "--gssep", gsSep());
        }

        if (isExtendable() && guardDescent() != 5.0f) {
            arg_float(cmd, "--guarddescent=", guardDescent(), true /*allowZero*/);
        }

        if (!autoHeight && !isFixedRatio()) {
            if (inputMode() & HEIGHTPERROW_MODE) {
                arg_float(cmd, "--height=", heightPerRow ? heightPerRow : height());
                arg_bool(cmd, "--heightperrow", true);
            } else {
                arg_float(cmd, "--height=", height());
            }
        }

        arg_bool(cmd, "--init", supportsReaderInit() && readerInit());

        if (hasMask()) {
            arg_int(cmd, "--mask=", (option3() >> 8) - 1, true /*allowZero*/);
        }

        if (m_symbol == BARCODE_MAXICODE || isComposite()) {
            arg_int(cmd, "--mode=", option1());
        }

        arg_bool(cmd, "--nobackground", bgColor() == QColor(0xFF, 0xFF, 0xFF, 0));
        arg_bool(cmd, "--noquietzones", hasDefaultQuietZones() && noQuietZones());
        arg_bool(cmd, "--notext", hasHRT() && !showText());
        arg_data(cmd, longOptOnly ? "--output=" : "-o ", outfile, win);

        if (m_symbol == BARCODE_MAXICODE || isComposite()) {
            arg_data(cmd, "--primary=", primaryMessage(), win);
        }

        arg_bool(cmd, "--quietzones", !hasDefaultQuietZones() && quietZones());
        arg_int(cmd, "--rotate=", rotateAngle());

        if (m_symbol == BARCODE_CODE16K || m_symbol == BARCODE_CODABLOCKF || m_symbol == BARCODE_HIBC_BLOCKF
                || m_symbol == BARCODE_CODE49) {
            arg_int(cmd, "--rows=", option1());
        } else if (m_symbol == BARCODE_DBAR_EXPSTK || m_symbol == BARCODE_DBAR_EXPSTK_CC
                || m_symbol == BARCODE_PDF417 || m_symbol == BARCODE_PDF417COMP || m_symbol == BARCODE_HIBC_PDF) {
            arg_int(cmd, "--rows=", option3());
        }

        if (scale() != 1.0f) {
            arg_float(cmd, "--scale=", scale());
        }

        if (m_symbol == BARCODE_MAXICODE) {
            arg_int(cmd, "--scmvv=", option2() - 1, true /*allowZero*/);
        }

        if (m_symbol == BARCODE_PDF417 || m_symbol == BARCODE_PDF417COMP || m_symbol == BARCODE_HIBC_PDF
                || m_symbol == BARCODE_AZTEC || m_symbol == BARCODE_HIBC_AZTEC
                || m_symbol == BARCODE_QRCODE || m_symbol == BARCODE_HIBC_QR || m_symbol == BARCODE_MICROQR
                || m_symbol == BARCODE_RMQR || m_symbol == BARCODE_GRIDMATRIX || m_symbol == BARCODE_HANXIN
                || m_symbol == BARCODE_ULTRA) {
            arg_int(cmd, "--secure=", option1());
        }

        if (m_symbol == BARCODE_CODE16K || m_symbol == BARCODE_CODABLOCKF || m_symbol == BARCODE_HIBC_BLOCKF
                || m_symbol == BARCODE_CODE49) {
            arg_int(cmd, "--separator=", option3());
        }

        arg_bool(cmd, "--small", fontSetting() & SMALL_TEXT);

        if (m_symbol == BARCODE_DATAMATRIX || m_symbol == BARCODE_HIBC_DM) {
            arg_bool(cmd, "--square", option3() == DM_SQUARE);
        }

        if (supportsStructApp()) {
            arg_structapp(cmd, "--structapp=", structAppCount(), structAppIndex(), structAppID(), win);
        }

        arg_bool(cmd, "--verbose", debug());

        if (m_symbol == BARCODE_AZTEC || m_symbol == BARCODE_HIBC_AZTEC
                || m_symbol == BARCODE_MSI_PLESSEY || m_symbol == BARCODE_CODE11
                || m_symbol == BARCODE_C25STANDARD || m_symbol == BARCODE_C25INTER || m_symbol == BARCODE_C25IATA
                || m_symbol == BARCODE_C25LOGIC || m_symbol == BARCODE_C25IND
                || m_symbol == BARCODE_CODE39 || m_symbol == BARCODE_HIBC_39 || m_symbol == BARCODE_EXCODE39
                || m_symbol == BARCODE_LOGMARS || m_symbol == BARCODE_CODABAR
                || m_symbol == BARCODE_DATAMATRIX || m_symbol == BARCODE_HIBC_DM
                || m_symbol == BARCODE_QRCODE || m_symbol == BARCODE_HIBC_QR || m_symbol == BARCODE_MICROQR
                || m_symbol == BARCODE_RMQR || m_symbol == BARCODE_GRIDMATRIX || m_symbol == BARCODE_HANXIN
                || m_symbol == BARCODE_CHANNEL || m_symbol == BARCODE_CODEONE || m_symbol == BARCODE_CODE93
                || m_symbol == BARCODE_ULTRA || m_symbol == BARCODE_VIN) {
            arg_int(cmd, "--vers=", option2());
        } else if (m_symbol == BARCODE_DAFT && option2() != 250) {
            arg_int(cmd, "--vers=", option2());
        }

        arg_int(cmd, "--vwhitesp=", vWhitespace());
        arg_int(cmd, longOptOnly ? "--whitesp=" : "-w ", whitespace());
        arg_bool(cmd, "--werror", warnLevel() == WARN_FAIL_ALL);

        return cmd;
    }

    /* `getAsCLI()` helpers */
    void QZint::arg_str(QString &cmd, const char *const opt, const QString &val) {
        if (!val.isEmpty()) {
            QByteArray bstr = val.toUtf8();
            cmd += QString::asprintf(" %s%.*s", opt, bstr.length(), bstr.data());
        }
    }

    void QZint::arg_int(QString &cmd, const char *const opt, const int val, const bool allowZero) {
        if (val > 0 || (val == 0 && allowZero)) {
            cmd += QString::asprintf(" %s%d", opt, val);
        }
    }

    void QZint::arg_bool(QString &cmd, const char *const opt, const bool val) {
        if (val) {
            cmd += QString::asprintf(" %s", opt);
        }
    }

    void QZint::arg_color(QString &cmd, const char *const opt, const QColor val) {
        if (val.alpha() != 0xFF) {
            cmd += QString::asprintf(" %s%02X%02X%02X%02X", opt, val.red(), val.green(), val.blue(), val.alpha());
        } else {
            cmd += QString::asprintf(" %s%02X%02X%02X", opt, val.red(), val.green(), val.blue());
        }
    }

    void QZint::arg_data(QString &cmd, const char *const opt, const QString &val, const bool win) {
        if (!val.isEmpty()) {
            QString text(val);
            const char delim = win ? '"' : '\'';
            if (win) {
                // Difficult (impossible?) to fully escape strings on Windows, e.g. "blah%PATH%" will substitute
                // env var PATH, so just doing basic escaping here
                text.replace("\\\\", "\\\\\\\\"); // Double-up backslashed backslash `\\` -> `\\\\`
                text.replace("\"", "\\\""); // Backslash quote `"` -> `\"`
                QByteArray bstr = text.toUtf8();
                cmd += QString::asprintf(" %s%c%.*s%c", opt, delim, bstr.length(), bstr.data(), delim);
            } else {
                text.replace("'", "'\\''"); // Single quote `'` -> `'\''`
                QByteArray bstr = text.toUtf8();
                cmd += QString::asprintf(" %s%c%.*s%c", opt, delim, bstr.length(), bstr.data(), delim);
            }
        }
    }

    void QZint::arg_float(QString &cmd, const char *const opt, const float val, const bool allowZero) {
        if (val > 0 || (val == 0 && allowZero)) {
            cmd += QString::asprintf(" %s%g", opt, val);
        }
    }

    void QZint::arg_structapp(QString &cmd, const char *const opt, const int count, const int index,
                                const QString &id, const bool win) {
        if (count >= 2 && index >= 1) {
            if (id.isEmpty()) {
                cmd += QString::asprintf(" %s%d,%d", opt, index, count);
            } else {
                QByteArray bstr = id.toUtf8();
                arg_data(cmd, opt, QString::asprintf("%d,%d,%.*s", index, count, bstr.length(), bstr.data()), win);
            }
        }
    }
} /* namespace Zint */
