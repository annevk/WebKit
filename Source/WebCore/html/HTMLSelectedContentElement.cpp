/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "HTMLSelectedContentElement.h"

#include "HTMLElement.h"
#include "HTMLNames.h"
#include "HTMLOptionElement.h"
#include "HTMLSelectElement.h"
#include <wtf/TZoneMallocInlines.h>

namespace WebCore {

WTF_MAKE_TZONE_OR_ISO_ALLOCATED_IMPL(HTMLSelectedContentElement);

using namespace HTMLNames;

HTMLSelectedContentElement::HTMLSelectedContentElement(Document& document)
    : HTMLElement(selectedcontentTag, document, { })
{
    ASSERT(hasTagName(selectedcontentTag));
}

Ref<HTMLSelectedContentElement> HTMLSelectedContentElement::create(const QualifiedName&, Document& document)
{
    return adoptRef(*new HTMLSelectedContentElement(document));
}

auto HTMLSelectedContentElement::insertedIntoAncestor(InsertionType insertionType, ContainerNode& parentOfInsertedTree) -> InsertedIntoAncestorResult
{
    HTMLElement::insertedIntoAncestor(insertionType, parentOfInsertedTree);

    ASSERT(document().settings().htmlEnhancedSelectParsingEnabled());
    ASSERT(document().settings().htmlEnhancedSelectSelectedContentEnabled());

    if (insertionType.connectedToDocument)
        return InsertedIntoAncestorResult::NeedsPostInsertionCallback;
    return InsertedIntoAncestorResult::Done;
}

void HTMLSelectedContentElement::didFinishInsertingNode()
{
    RefPtr<HTMLSelectElement> nearestAncestorSelect;
    m_isDisabled = false;
    for (Ref ancestor : ancestorsOfType<HTMLElement>(*this)) {
        if (RefPtr select = dynamicDowncast<HTMLSelectElement>(ancestor)) {
            if (!nearestAncestorSelect)
                nearestAncestorSelect = WTFMove(select);
            else
                m_isDisabled = true;
            break;
        }
        if (is<HTMLOptionElement>(ancestor) || is<HTMLSelectedContentElement>(ancestor)) {
            m_isDisabled = true;
            break;
        }
    }
    if (!nearestAncestorSelect || nearestAncestorSelect->multiple())
        return;
    nearestAncestorSelect->updateSelectedContent();
    nearestAncestorSelect->clearNonPrimarySelectedContent();
}

void HTMLSelectedContentElement::removedFromAncestor(RemovalType removalType, ContainerNode& oldParentOfRemovedTree)
{
    HTMLElement::removedFromAncestor(removalType, oldParentOfRemovedTree);

    ASSERT(document().settings().htmlEnhancedSelectParsingEnabled());
    ASSERT(document().settings().htmlEnhancedSelectSelectedContentEnabled());

    if (ancestorsOfType<HTMLSelectElement>(*this).first())
        return;

    // FIXME: inclusiveAncestorsOfType would be nice.
    if (RefPtr select = dynamicDowncast<HTMLSelectElement>(oldParentOfRemovedTree)) {
        select->updateSelectedContent();
        return;
    }

    if (RefPtr select = ancestorsOfType<HTMLSelectElement>(oldParentOfRemovedTree).first()) {
        select->updateSelectedContent();
        return;
    }
}

} // namespace WebCore
