/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2011, 2012 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "html/HTMLNameCollection.h"

#include "dom/Element.h"
#include "html/HTMLDocument.h"
#include "html/HTMLFormElement.h"
#include "html/HTMLImageElement.h"
#include "HTMLNames.h"
#include "dom/NodeRareData.h"
#include "dom/NodeTraversal.h"

namespace WebCore {

using namespace HTMLNames;

HTMLNameCollection::HTMLNameCollection(Node* document, CollectionType type, const AtomicString& name)
    : HTMLCollection(document, type, DoesNotOverrideItemAfter)
    , m_name(name)
{
}

HTMLNameCollection::~HTMLNameCollection()
{
    ASSERT(ownerNode());
    ASSERT(ownerNode()->isDocumentNode());
    ASSERT(type() == WindowNamedItems || type() == DocumentNamedItems);

    ownerNode()->nodeLists()->removeCacheWithAtomicName(this, type(), m_name);
}

bool WindowNameCollection::nodeMatchesIfNameAttributeMatch(Element* element)
{
    return isHTMLImageElement(element) || isHTMLFormElement(element); 
}

bool WindowNameCollection::nodeMatches(Element* element, const AtomicString& name)
{
    // Find only images, forms, and objects by name, but anything by id
    if (nodeMatchesIfNameAttributeMatch(element) && element->getNameAttribute() == name)
        return true;
    return element->getIdAttribute() == name;
}

bool DocumentNameCollection::nodeMatchesIfIdAttributeMatch(Element* element)
{
    // FIXME: we need to fix HTMLImageElement to update the hash map for us when name attribute has been removed.
    return isHTMLImageElement(element) && element->hasName();
}

bool DocumentNameCollection::nodeMatchesIfNameAttributeMatch(Element* element)
{
    return isHTMLFormElement(element) || element->hasTagName(iframeTag) || isHTMLImageElement(element);
}

bool DocumentNameCollection::nodeMatches(Element* element, const AtomicString& name)
{
    // Find images, forms, objects and iframes by name, applets and object by id, and images by id
    // but only if they have a name attribute (this very strange rule matches IE)
    if (isHTMLFormElement(element) || element->hasTagName(iframeTag))
        return element->getNameAttribute() == name;

    if (isHTMLImageElement(element))
        return element->getNameAttribute() == name || (element->getIdAttribute() == name && element->hasName());
    return false;
}

}
