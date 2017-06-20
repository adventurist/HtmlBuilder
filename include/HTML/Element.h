/**
 * @file    Element.h
 * @ingroup HtmlBuilder
 * @brief   Definitions of an Element in the HTML Document Object Model, and various specialized Element types.
 *
 * Copyright (c) 2017 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>


/// A simple C++ HTML Generator library.
namespace HTML {

#ifndef HTML_INDENTATION
#define HTML_INDENTATION 2
#endif

/**
 * @brief Definitions of an Element in the HTML Document Object Model, and various specialized Element types.
 *
 * An Element represents any HTML node in the Document Object Model.
 */
class Element {
public:
    Element(const char* apName, std::string&& aContent) :
        mName(apName), mContent(aContent) {}
    Element(const char* apName, const std::string& aContent) :
        mName(apName), mContent(aContent) {}
    explicit Element(const char* apName, const char* apContent = nullptr) :
        mName(apName), mContent(apContent ? apContent : "") {}

    Element&& addAttribute(const char* apName, const std::string& aValue) {
        mAttributes[apName] = aValue;
        return std::move(*this);
    }
    Element&& addAttribute(const char* apName, const unsigned int aValue) {
        mAttributes[apName] = std::to_string(aValue);
        return std::move(*this);
    }
    Element&& operator<<(Element&& aElement) {
        mChildren.push_back(std::move(aElement));
        return std::move(*this);
    }
    Element&& operator<<(const char* apContent);
    Element&& operator<<(std::string&& aContent);
    Element&& operator<<(const std::string& aContent);

    friend std::ostream& operator<<(std::ostream& aStream, const Element& aElement);
    std::string toString() const {
        std::ostringstream stream;
        stream << *this;
        return stream.str();
    }

    Element&& id(const std::string& aValue) {
        return addAttribute("id", aValue);
    }

    Element&& cls(const std::string& aValue) {
        return addAttribute("class", aValue);
    }

    Element&& title(const std::string& aValue) {
        return addAttribute("title", aValue);
    }

    Element&& style(const std::string& aValue) {
        return addAttribute("style", aValue);
    }

protected:
    /// Constructor reserved for the Root \<html\> Element
    Element();

    std::ostream& toString(std::ostream& aStream, const size_t aIndentation = 0) const {
        toStringOpen(aStream, aIndentation);
        toStringContent(aStream, aIndentation);
        toStringClose(aStream, aIndentation);
        return aStream;
    }

private:
    void toStringOpen(std::ostream& aStream, const size_t aIndentation) const {
        if (!mName.empty()) {
            std::fill_n(std::ostream_iterator<char>(aStream), aIndentation, ' ');
            aStream << '<' << mName;

            for (const auto& attr : mAttributes) {
                aStream << ' ' << attr.first;
                if (!attr.second.empty()) {
                    aStream << "=\"" << attr.second << "\"";
                }
            }

            if (mContent.empty()) {
                if (mbNonVoid) {
                    aStream << ">";
                } else if (!mChildren.empty()) {
                    aStream << ">\n";
                } else {
                    aStream << "/>\n";
                }
            } else {
                aStream << '>';
            }
        }
    }
    void toStringContent(std::ostream& aStream, const size_t aIndentation) const {
        if (!mName.empty()) {
            aStream << mContent;
            for (auto& child : mChildren) {
                child.toString(aStream, aIndentation + HTML_INDENTATION);
            }
        } else {
            std::fill_n(std::ostream_iterator<char>(aStream), aIndentation, ' ');
            aStream << mContent << '\n';
        }
    }
    void toStringClose(std::ostream& aStream, const size_t aIndentation) const {
        if (!mName.empty()) {
            if (!mChildren.empty()) {
                std::fill_n(std::ostream_iterator<char>(aStream), aIndentation, ' ');
            }
            if (!mContent.empty() || !mChildren.empty() || mbNonVoid) {
                aStream << "</" << mName << ">\n";
            }
        }
    }

protected:
    std::string mName;
    std::string mContent;
    std::map<std::string, std::string> mAttributes;
    std::vector<Element> mChildren;
    bool mbNonVoid = false; // ex. <td></td> (<td/> is not allowed)
};

inline std::ostream& operator<<(std::ostream& aStream, const Element& aElement) {
    return aElement.toString(aStream);
}

/// Raw content text (unnamed Element) to use as text values between child Elements
class Text : public Element {
public:
    explicit Text(const char* apContent) : Element("", apContent) {}
    explicit Text(std::string&& aContent) : Element("", aContent) {}
    explicit Text(const std::string& aContent) : Element("", aContent) {}
};

inline Element&& Element::operator<<(const char* apContent) {
    return *this << Text(apContent);
}

inline Element&& Element::operator<<(std::string&& aContent) {
    return *this << Text(std::move(aContent));
}

inline Element&& Element::operator<<(const std::string& aContent) {
    return *this << Text(aContent);
}

/// \<title\> Element required in \<head\>
class Title : public Element {
public:
    explicit Title(const char* apContent) : Element("title", apContent) {}
    explicit Title(const std::string& aContent) : Element("title", aContent) {}
};

/// \<style\> Element for inline CSS in \<head\>
class Style : public Element {
public:
    explicit Style(const char* apContent) : Element("style", apContent) {}
    explicit Style(const std::string& aContent) : Element("style", aContent) {}
};

/// \<script\> Element for inline Javascript in \<head\>
class Script : public Element {
public:
    explicit Script(const char* apSrc, const char* apContent = nullptr) : Element("script", apContent) {
        if (nullptr != apSrc) {
            addAttribute("src", apSrc);
        }
    }
};

/// \<meta\> Element in \<head\>
class Meta : public Element {
public:
    explicit Meta(const char* apCharset) : Element("meta") {
        addAttribute("charset", apCharset);
    }
    explicit Meta(const char* apName, const char* apContent) : Element("meta") {
        addAttribute("name", apName);
        addAttribute("content", apContent);
    }
};

/// \<link\> Element to reference external CSS or Javascript files in \<head\>
class Rel : public Element {
public:
    Rel(const char* apRel, const char* apUrl, const char* apType = nullptr) : Element("link") {
        addAttribute("rel", apRel);
        addAttribute("href", apUrl);
        if (nullptr != apType) {
            addAttribute("type", apType);
        }
    }
};

/// \<base\> Element in \<head\>
class Base : public Element {
public:
    Base(const std::string& aContent, const std::string& aUrl, const char* apTarget) : Element("base", aContent) {
        addAttribute("href", aUrl);
        if (nullptr != apTarget) {
            addAttribute("target", apTarget);
        }
    }
};

/// \<head\> required as the first child Element in every HTML Document
class Head : public Element {
public:
    Head() : Element("head") {}

    Head&& operator<<(Element&& aElement) = delete;
    Head&& operator<<(Title&& aTitle) {
        mChildren.push_back(std::move(aTitle));
        return std::move(*this);
    }
    Head&& operator<<(Style&& aStyle) {
        mChildren.push_back(std::move(aStyle));
        return std::move(*this);
    }
    Head&& operator<<(Script&& aScript) {
        mChildren.push_back(std::move(aScript));
        return std::move(*this);
    }
    Head&& operator<<(Meta&& aMeta) {
        mChildren.push_back(std::move(aMeta));
        return std::move(*this);
    }
    Head&& operator<<(Rel&& aRel) {
        mChildren.push_back(std::move(aRel));
        return std::move(*this);
    }
    Head&& operator<<(Base&& aBase) {
        mChildren.push_back(std::move(aBase));
        return std::move(*this);
    }
};

/// \<body\> required as the second child Element in every HTML Document
class Body : public Element {
public:
    Body() : Element("body") {}
};

// Constructor of the Root \<html\> Element
inline Element::Element() : mName("html"), mChildren{Head(), Body()} {
}


/// \<br\> Line break Element
class Break : public Element {
public:
    Break() : Element("br") {}
};

/// \<th\> Table Header Column Element
class ColHeader : public Element {
public:
    explicit ColHeader(const char* apContent = nullptr) : Element("th", apContent) {
        mbNonVoid = true;
    }
    explicit ColHeader(std::string&& aContent) : Element("th", aContent) {
        mbNonVoid = true;
    }
    explicit ColHeader(const std::string& aContent) : Element("th", aContent) {
        mbNonVoid = true;
    }

    ColHeader&& rowSpan(const unsigned int aNbRow) {
        if (0 < aNbRow) {
            addAttribute("rowspan", aNbRow);
        }
        return std::move(*this);
    }
    ColHeader&& colSpan(const unsigned int aNbCol) {
        if (0 < aNbCol) {
            addAttribute("colspan", aNbCol);
        }
        return std::move(*this);
    }
};

/// \<td\> Table Column Element
class Col : public Element {
public:
    explicit Col(const char* apContent = nullptr) : Element("td", apContent) {
        mbNonVoid = true;
    }
    explicit Col(std::string&& aContent) : Element("td", aContent) {
        mbNonVoid = true;
    }
    explicit Col(const std::string& aContent) : Element("td", aContent) {
        mbNonVoid = true;
    }

    Col&& rowSpan(const unsigned int aNbRow) {
        if (0 < aNbRow) {
            addAttribute("rowspan", aNbRow);
        }
        return std::move(*this);
    }
    Col&& colSpan(const unsigned int aNbCol) {
        if (0 < aNbCol) {
            addAttribute("colspan", aNbCol);
        }
        return std::move(*this);
    }
};

/// \<tr\> Table Row Element
class Row : public Element {
public:
    Row() : Element("tr") {}

    Row&& operator<<(Element&& aElement) = delete;
    Row&& operator<<(ColHeader&& aCol) {
        mChildren.push_back(std::move(aCol));
        return std::move(*this);
    }
    Row&& operator<<(Col&& aCol) {
        mChildren.push_back(std::move(aCol));
        return std::move(*this);
    }
};

/// \<table\> Element
class Table : public Element {
public:
    Table() : Element("table") {}

    Element&& operator<<(Element&& aElement) = delete;
    Element&& operator<<(Row&& aRow) {
        mChildren.push_back(std::move(aRow));
        return std::move(*this);
    }
};

/// \<ol\> Ordered List or \<ul\> Unordered List Element to use with ListItem
class List : public Element {
public:
    explicit List(const bool abOrdered = false) : Element(abOrdered?"ol":"ul") {}
};

/// \<li\> List Item Element to put in List
class ListItem : public Element {
public:
    explicit ListItem(const char* apContent = nullptr) : Element("li", apContent) {}
    explicit ListItem(const std::string& aContent) : Element("li", aContent) {}
};

/// \<form\> Element
class Form : public Element {
public:
    explicit Form(const char* apAction = nullptr) : Element("form") {
        if (nullptr != apAction) {
            addAttribute("action", apAction);
        }
    }
};

/// \<input\> Element to use in Form
class Input : public Element {
public:
    explicit Input(const char* apType = nullptr, const char* apName = nullptr,
                   const char* apValue = nullptr, const char* apContent = nullptr) : Element("input", apContent) {
        if (nullptr != apType) {
            addAttribute("type", apType);
        }
        if (nullptr != apName) {
            addAttribute("name", apName);
        }
        if (nullptr != apValue) {
            addAttribute("value", apValue);
        }
    }

    Input&& addAttribute(const char* apName, const std::string& aValue) {
        Element::addAttribute(apName, aValue);
        return std::move(*this);
    }
    Input&& addAttribute(const char* apName, const unsigned int aValue) {
        Element::addAttribute(apName, aValue);
        return std::move(*this);
    }

    Input&& id(const std::string& aValue) {
        return addAttribute("id", aValue);
    }
    Input&& cls(const std::string& aValue) {
        return addAttribute("class", aValue);
    }
    Input&& title(const std::string& aValue) {
        return addAttribute("title", aValue);
    }
    Input&& style(const std::string& aValue) {
        return addAttribute("style", aValue);
    }

    Input&& size(const unsigned int aSize) {
        return addAttribute("size", aSize);
    }
    Input&& maxlength(const unsigned int aMaxlength) {
        return addAttribute("maxlength", aMaxlength);
    }
    Input&& placeholder(const std::string& aPlaceholder) {
        return addAttribute("placeholder", aPlaceholder);
    }
    Input&& min(const std::string& aMin) {
        return addAttribute("min", aMin);
    }
    Input&& min(const unsigned int aMin) {
        return addAttribute("min", aMin);
    }
    Input&& max(const std::string& aMax) { // NOLINT(build/include_what_you_use) false positive
        return addAttribute("max", aMax);
    }
    Input&& max(const unsigned int aMax) { // NOLINT(build/include_what_you_use) false positive
        return addAttribute("max", aMax);
    }

    Input&& checked(const bool abChecked = true) {
        if (abChecked) {
            addAttribute("checked", "");
        }
        return std::move(*this);
    }
    Input&& autocomplete() {
        return addAttribute("autocomplete", "");
    }
    Input&& autofocus() {
        return addAttribute("autofocus", "");
    }
    Input&& disabled() {
        return addAttribute("disabled", "");
    }
    Input&& readonly() {
        return addAttribute("readonly", "");
    }
    Input&& required() {
        return addAttribute("required", "");
    }
};

/// \<input\> Radio Element to use in Form
class InputRadio : public Input {
public:
    explicit InputRadio(const char* apName, const char* apValue = nullptr, const char* apContent = nullptr) :
        Input("radio", apName, apValue, apContent) {
    }
};

/// \<input\> Checkbox Element to use in Form
class InputCheckbox : public Input {
public:
    explicit InputCheckbox(const char* apName, const char* apValue = nullptr, const char* apContent = nullptr) :
        Input("checkbox", apName, apValue, apContent) {
    }
};

/// \<input\> text Element to use in Form
class InputText : public Input {
public:
    explicit InputText(const char* apName, const char* apValue = nullptr) :
        Input("text", apName, apValue) {
    }
};

/// \<textarea\> Element to use in Form
class TextArea : public Element {
public:
    explicit TextArea(const char* apName, const unsigned int aCols = 0, const unsigned int aRows = 0) :
        Element("textarea") {
        addAttribute("name", apName);
        if (0 < aCols) {
            addAttribute("cols", aCols);
        }
        if (0 < aRows) {
            addAttribute("rows", aRows);
        }
        mbNonVoid = true;
    }
    TextArea&& maxlength(const unsigned int aMaxlength) {
        addAttribute("maxlength", aMaxlength);
        return std::move(*this);
    }
};

/// \<intput\> Number Element to use in Form
class InputNumber : public Input {
public:
    explicit InputNumber(const char* apName, const char* apValue = nullptr) :
        Input("number", apName, apValue) {
    }
};

/// \<intput\> Range Element to use in Form
class InputRange : public Input {
public:
    explicit InputRange(const char* apName, const char* apValue = nullptr) :
        Input("range", apName, apValue) {
    }
};

/// \<intput\> Date Element to use in Form
class InputDate : public Input {
public:
    explicit InputDate(const char* apName, const char* apValue = nullptr) :
        Input("date", apName, apValue) {
    }
};

/// \<intput\> Time Element to use in Form
class InputTime : public Input {
public:
    explicit InputTime(const char* apName, const char* apValue = nullptr) :
        Input("time", apName, apValue) {
    }
};

/// \<intput\> E-mail Element to use in Form
class InputEmail : public Input {
public:
    explicit InputEmail(const char* apName, const char* apValue = nullptr) :
        Input("email", apName, apValue) {
    }
};

/// \<intput\> URL Element to use in Form
class InputUrl : public Input {
public:
    explicit InputUrl(const char* apName, const char* apValue = nullptr) :
        Input("url", apName, apValue) {
    }
};

/// \<intput\> Password Element to use in Form
class InputPassword : public Input {
public:
    explicit InputPassword(const char* apName) :
        Input("password", apName) {
    }
};

/// \<intput\> Submit Button Element to use in Form
class InputSubmit : public Input {
public:
    explicit InputSubmit(const char* apValue = nullptr, const char* apName = nullptr) :
        Input("submit", apName, apValue) {
    }
};

/// \<intput\> Reset Button Element to use in Form
class InputReset : public Input {
public:
    explicit InputReset(const char* apValue = nullptr) :
        Input("reset", nullptr, apValue) {
    }
};

/// \<intput\> List Element to use in Form with DataList
class InputList : public Input {
public:
    explicit InputList(const char* apName, const char* apList) : Input(nullptr, apName) {
        addAttribute("list", apList);
    }
};

/// \<datalist\> Element for InputList, to use with Option Elements
class DataList : public Element {
public:
    explicit DataList(const char* apId) : Element("datalist") {
        addAttribute("id", apId);
    }
};

/// \<select\> Element to use with Option Elements
class Select : public Element {
public:
    explicit Select(const char* apName) : Element("select") {
        addAttribute("name", apName);
    }
};

/// \<option\> Element for Select and DataList
class Option : public Element {
public:
    explicit Option(const char* apValue, const char* apContent = nullptr) : Element("option", apContent) {
        addAttribute("value", apValue);
        mbNonVoid = true;
    }

    Option&& selected(const bool abSelected = true) {
        if (abSelected) {
            addAttribute("selected", "");
        }
        return std::move(*this);
    }
};

/// \<h1\> Element
class Header1 : public Element {
public:
    explicit Header1(const std::string& aContent) : Element("h1", aContent) {}
};

/// \<h2\> Element
class Header2 : public Element {
public:
    explicit Header2(const std::string& aContent) : Element("h2", aContent) {}
};

/// \<h3\> Element
class Header3 : public Element {
public:
    explicit Header3(const std::string& aContent) : Element("h3", aContent) {}
};

/// \<b\> Element
class Bold : public Element {
public:
    explicit Bold(const std::string& aContent) : Element("b", aContent) {}
};

/// \<i\> Element
class Italic : public Element {
public:
    explicit Italic(const std::string& aContent) : Element("i", aContent) {}
};

/// \<strong\> Element
class Strong : public Element {
public:
    explicit Strong(const std::string& aContent) : Element("strong", aContent) {}
};

/// \<p\> Element
class Paragraph : public Element {
public:
    explicit Paragraph(const std::string& aContent) : Element("p", aContent) {}
};

/// \<div\> Element
class Div : public Element {
public:
    explicit Div(const std::string& aContent) : Element("div", aContent) {}
};

/// \<span\> Element
class Span : public Element {
public:
    explicit Span(const std::string& aContent) : Element("span", aContent) {}
};

/// \<a\> Hyper-Link Element
class Link : public Element {
public:
    Link(const std::string& aContent, const std::string& aUrl) : Element("a", aContent) {
        addAttribute("href", aUrl);
    }
};

/// \<img\> Image Element
class Image : public Element {
public:
    Image(const std::string& aSrc, const std::string& aAlt, unsigned int aWidth = 0, unsigned int aHeight = 0) :
        Element("img") {
        addAttribute("src", aSrc);
        addAttribute("alt", aAlt);
        if (0 < aWidth) {
            addAttribute("width", aWidth);
        }
        if (0 < aHeight) {
            addAttribute("height", aHeight);
        }
    }
};

/// \<mark\> semantic Element
class Mark : public Element {
public:
    explicit Mark(const std::string& aContent) : Element("mark", aContent) {}
};

/// \<time\> semantic Element
class Time : public Element {
public:
    explicit Time(const std::string& aContent, const std::string& aDateTime) : Element("time", aContent) {
        addAttribute("datetime", aDateTime);
    }
};

/// \<header\> semantic Element
class Header : public Element {
public:
    Header() : Element("header") {}
};

/// \<footer\> semantic Element
class Footer : public Element {
public:
    Footer() : Element("footer") {}
};

/// \<section\> semantic Element
class Section : public Element {
public:
    Section() : Element("section") {}
};

/// \<article\> semantic Element
class Article : public Element {
public:
    Article() : Element("article") {}
};

/// \<nav\> semantic Element
class Nav : public Element {
public:
    Nav() : Element("nav") {}
};

/// \<aside\> semantic Element
class Aside : public Element {
public:
    Aside() : Element("aside") {}
};

/// \<main\> semantic Element
class Main : public Element {
public:
    Main() : Element("main") {}
};

/// \<figure\> semantic Element
class Figure : public Element {
public:
    Figure() : Element("figure") {}
};

/// \<figcaption\> semantic Element to use with Figure
class FigCaption : public Element {
public:
    explicit FigCaption(const std::string& aContent) : Element("figcaption", aContent) {}
};

/** @brief \<details\> semantic Element containing detailed information to use with Summary.
 *
 * @verbatim
<details>
  <summary>Copyright 2017.</summary>
  <p>By Sébastien Rombauts.</p>
  <p>sebastien.rombauts@gmail.com.</p>
</details> @endverbatim
 */
class Details : public Element {
public:
    Details(const char* apOpen = nullptr) : Element("details") {
        if (nullptr != apOpen) {
            addAttribute("open", apOpen);
        }
    }
};

/// \<summary\> semantic Element to use inside a Details section to specify a visible heading
class Summary : public Element {
public:
    explicit Summary(const std::string& aContent) : Element("summary", aContent) {}
};


} // namespace HTML
