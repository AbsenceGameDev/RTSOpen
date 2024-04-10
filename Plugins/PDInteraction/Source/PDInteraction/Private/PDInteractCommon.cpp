/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#include "PDInteractCommon.h"

void FPDTraceBuffer::Setup()
{
	Frames.Empty();
	AddTraceFrame(EPDTraceResult::TRACE_FAIL, FHitResult(ForceInit));
}

const FPDTraceResult& FPDTraceBuffer::GetLastTraceResult() const
{
	check(Frames.Num() > 0);
	return Frames[0];
}

const FPDTraceResult& FPDTraceBuffer::GetLastValidResult() const
{
	return CachedValidFrame;
}

const bool FPDTraceBuffer::HasValidResults() const
{
	const bool bValidCachedFrameStillRelevant = ValidFrameResetIt <= (FrameResetLimit / 2) ;
	return bValidCachedFrameStillRelevant && CachedValidFrame.ResultFlag == EPDTraceResult::TRACE_SUCCESS && CachedValidFrame.HitResult.GetActor() != nullptr;
}

void FPDTraceBuffer::ClearTraceResults()
{
	Frames.Empty();
}

void FPDTraceBuffer::AddTraceFrame(EPDTraceResult TraceResult, const FHitResult& HitResult)
{
	if (Frames.Num() > 10)
	{
		Frames.Empty();
		ValidFrameResetIt++;
	}

	if ((ValidFrameResetIt % FrameResetLimit) == 0) // clear each 'FrameResetLimit'
	{
		CachedValidFrame = FPDTraceResult{EPDTraceResult::TRACE_FAIL, FHitResult(ForceInit)};
		ValidFrameResetIt = 1; // Don't want it to overflow
	}
		
	Frames.EmplaceFirst(FPDTraceResult{TraceResult, HitResult});
	if (TraceResult != EPDTraceResult::TRACE_SUCCESS) { return; }

	CachedValidFrame = FPDTraceResult{TraceResult, HitResult};
}


/*
 * @copyright Permafrost Development (MIT license)
 * Authors: Ario Amin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */