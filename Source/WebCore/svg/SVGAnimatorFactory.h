/*
 * Copyright (C) Research In Motion Limited 2011, 2012. All rights reserved.
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
 */

#ifndef SVGAnimatorFactory_h
#define SVGAnimatorFactory_h

#if ENABLE(SVG)
#include "svg/SVGAnimatedAngle.h"
#include "svg/SVGAnimatedBoolean.h"
#include "svg/SVGAnimatedColor.h"
#include "svg/SVGAnimatedEnumeration.h"
#include "svg/SVGAnimatedInteger.h"
#include "svg/SVGAnimatedIntegerOptionalInteger.h"
#include "svg/SVGAnimatedLength.h"
#include "svg/SVGAnimatedLengthList.h"
#include "svg/SVGAnimatedNumber.h"
#include "svg/SVGAnimatedNumberList.h"
#include "svg/SVGAnimatedNumberOptionalNumber.h"
#include "svg/SVGAnimatedPath.h"
#include "svg/SVGAnimatedPointList.h"
#include "svg/SVGAnimatedPreserveAspectRatio.h"
#include "svg/SVGAnimatedRect.h"
#include "svg/SVGAnimatedString.h"
#include "svg/SVGAnimatedTransformList.h"

namespace WebCore {

class SVGAnimationElement;

class SVGAnimatorFactory {
public:
    static PassOwnPtr<SVGAnimatedTypeAnimator> create(SVGAnimationElement* animationElement, SVGElement* contextElement, AnimatedPropertyType attributeType)
    {
        ASSERT(animationElement);
        ASSERT(contextElement);

        switch (attributeType) {
        case AnimatedAngle:
            return adoptPtr(new SVGAnimatedAngleAnimator(animationElement, contextElement));
        case AnimatedBoolean:
            return adoptPtr(new SVGAnimatedBooleanAnimator(animationElement, contextElement));
        case AnimatedColor:
            return adoptPtr(new SVGAnimatedColorAnimator(animationElement, contextElement));
        case AnimatedEnumeration:
            return adoptPtr(new SVGAnimatedEnumerationAnimator(animationElement, contextElement));
        case AnimatedInteger:
            return adoptPtr(new SVGAnimatedIntegerAnimator(animationElement, contextElement));
        case AnimatedIntegerOptionalInteger:
            return adoptPtr(new SVGAnimatedIntegerOptionalIntegerAnimator(animationElement, contextElement));
        case AnimatedLength:
            return adoptPtr(new SVGAnimatedLengthAnimator(animationElement, contextElement));
        case AnimatedLengthList:
            return adoptPtr(new SVGAnimatedLengthListAnimator(animationElement, contextElement));
        case AnimatedNumber:
            return adoptPtr(new SVGAnimatedNumberAnimator(animationElement, contextElement));
        case AnimatedNumberList:
            return adoptPtr(new SVGAnimatedNumberListAnimator(animationElement, contextElement));
        case AnimatedNumberOptionalNumber:
            return adoptPtr(new SVGAnimatedNumberOptionalNumberAnimator(animationElement, contextElement));
        case AnimatedPath:
            return adoptPtr(new SVGAnimatedPathAnimator(animationElement, contextElement));
        case AnimatedPoints:
            return adoptPtr(new SVGAnimatedPointListAnimator(animationElement, contextElement));
        case AnimatedPreserveAspectRatio:
            return adoptPtr(new SVGAnimatedPreserveAspectRatioAnimator(animationElement, contextElement));
        case AnimatedRect:
            return adoptPtr(new SVGAnimatedRectAnimator(animationElement, contextElement));
        case AnimatedString:
            return adoptPtr(new SVGAnimatedStringAnimator(animationElement, contextElement));
        case AnimatedTransformList:
            return adoptPtr(new SVGAnimatedTransformListAnimator(animationElement, contextElement));
        case AnimatedUnknown:
            break;
        }

        ASSERT_NOT_REACHED();
        return nullptr;
    }

private:
    SVGAnimatorFactory() { }

};
    
} // namespace WebCore

#endif // ENABLE(SVG)
#endif // SVGAnimatorFactory_h
