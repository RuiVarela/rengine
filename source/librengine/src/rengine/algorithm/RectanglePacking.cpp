// __!!rengine_copyright!!__ //

#include <rengine/algorithm/RectanglePacking.h>

#include <utility>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <limits>
#include <memory.h>

using namespace std;

namespace rengine
{

	static Bool IsContainedIn(const BinPackRectangle &a, const BinPackRectangle &b)
	{
		return a.x >= b.x && a.y >= b.y 
			&& a.x+a.width <= b.x+b.width 
			&& a.y+a.height <= b.y+b.height;
	}

	GuillotineBinPack::GuillotineBinPack()
		:binWidth(0), binHeight(0)
	{
	}

	GuillotineBinPack::GuillotineBinPack(Int width, Int height)
	{
		Init(width, height);
	}

	void GuillotineBinPack::Init(Int width, Int height)
	{
		binWidth = width;
		binHeight = height;

		// Clear any memory of previously packed rectangles.
		usedRectangles.clear();

		// We start with a single big free rectangle that spans the whole bin.
		BinPackRectangle n;
		n.x = 0;
		n.y = 0;
		n.width = width;
		n.height = height;

		freeRectangles.clear();
		freeRectangles.push_back(n);
	}

	void GuillotineBinPack::Insert(BinPackRectangles &rects, BinPackRectangles &dst, Bool merge, FreeRectChoiceHeuristic rectChoice, GuillotineSplitHeuristic splitMethod)
	{
		dst.clear();

		// Remember variables about the best packing choice we have made so far during the iteration process.
		Int bestFreeRect = 0;
		Int bestRect = 0;
		Bool bestFlipped = false;

		// Pack rectangles one at a time until we have cleared the rects array of all rectangles.
		// rects will get destroyed in the process.
		while(rects.size() > 0)
		{
			// Stores the penalty score of the best rectangle placement - bigger=worse, smaller=better.
			Int bestScore = std::numeric_limits<Int>::max();

			for(size_t i = 0; i < freeRectangles.size(); ++i)
			{
				for(size_t j = 0; j < rects.size(); ++j)
				{
					// If this rectangle is a perfect match, we pick it instantly.
					if (rects[j].width == freeRectangles[i].width && rects[j].height == freeRectangles[i].height)
					{
						bestFreeRect = i;
						bestRect = j;
						bestFlipped = false;
						bestScore = std::numeric_limits<Int>::min();
						i = freeRectangles.size(); // Force a jump out of the outer loop as well - we got an instant fit.
						break;
					}
					// If flipping this rectangle is a perfect match, pick that then.
					else if (rects[j].height == freeRectangles[i].width && rects[j].width == freeRectangles[i].height)
					{
						bestFreeRect = i;
						bestRect = j;
						bestFlipped = true;
						bestScore = std::numeric_limits<Int>::min();
						i = freeRectangles.size(); // Force a jump out of the outer loop as well - we got an instant fit.
						break;
					}
					// Try if we can fit the rectangle upright.
					else if (rects[j].width <= freeRectangles[i].width && rects[j].height <= freeRectangles[i].height)
					{
						Int score = ScoreByHeuristic(rects[j].width, rects[j].height, freeRectangles[i], rectChoice);
						if (score < bestScore)
						{
							bestFreeRect = i;
							bestRect = j;
							bestFlipped = false;
							bestScore = score;
						}
					}
					// If not, then perhaps flipping sideways will make it fit?
					else if (rects[j].height <= freeRectangles[i].width && rects[j].width <= freeRectangles[i].height)
					{
						Int score = ScoreByHeuristic(rects[j].height, rects[j].width, freeRectangles[i], rectChoice);
						if (score < bestScore)
						{
							bestFreeRect = i;
							bestRect = j;
							bestFlipped = true;
							bestScore = score;
						}
					}
				}
			}

			// If we didn't manage to find any rectangle to pack, abort.
			if (bestScore == std::numeric_limits<Int>::max())
				return;

			// Otherwise, we're good to go and do the actual packing.
			BinPackRectangle newNode;
			newNode.x = freeRectangles[bestFreeRect].x;
			newNode.y = freeRectangles[bestFreeRect].y;
			newNode.width = rects[bestRect].width;
			newNode.height = rects[bestRect].height;

			if (bestFlipped)
				std::swap(newNode.width, newNode.height);

			// Remove the free space we lost in the bin.
			SplitFreeRectByHeuristic(freeRectangles[bestFreeRect], newNode, splitMethod);
			freeRectangles.erase(freeRectangles.begin() + bestFreeRect);

			// Remove the rectangle we just packed from the input list.
			rects.erase(rects.begin() + bestRect);

			// Perform a Rectangle Merge step if desired.
			if (merge)
				MergeFreeList();

			// Remember the new used rectangle.
			usedRectangles.push_back(newNode);
		}
	}

	/// @return True if r fits inside freeRect (possibly rotated).
	Bool Fits(const BinPackRectangle &r, const BinPackRectangle &freeRect)
	{
		return (r.width <= freeRect.width && r.height <= freeRect.height) || (r.height <= freeRect.width && r.width <= freeRect.height);
	}

	/// @return True if r fits perfectly inside freeRect, i.e. the leftover area is 0.
	Bool FitsPerfectly(const BinPackRectangle &r, const BinPackRectangle &freeRect)
	{
		return (r.width == freeRect.width && r.height == freeRect.height) || (r.height == freeRect.width && r.width == freeRect.height);
	}

	BinPackRectangle GuillotineBinPack::Insert(Int width, Int height, Bool merge, FreeRectChoiceHeuristic rectChoice, GuillotineSplitHeuristic splitMethod)
	{
		// Find where to put the new rectangle.
		Int freeNodeIndex = 0;
		BinPackRectangle newRect = FindPositionForNewNode(width, height, rectChoice, &freeNodeIndex);

		// Abort if we didn't have enough space in the bin.
		if (newRect.height == 0)
			return newRect;

		// Remove the space that was just consumed by the new rectangle.
		SplitFreeRectByHeuristic(freeRectangles[freeNodeIndex], newRect, splitMethod);
		freeRectangles.erase(freeRectangles.begin() + freeNodeIndex);

		// Perform a Rectangle Merge step if desired.
		if (merge)
			MergeFreeList();

		// Remember the new used rectangle.
		usedRectangles.push_back(newRect);

		return newRect;
	}

	/// Computes the ratio of used surface area to the total bin area.
	Real GuillotineBinPack::Occupancy() const
	{
		///\todo The occupancy rate could be cached/tracked incrementally instead
		///      of looping through the list of packed rectangles here.
		Ulong usedSurfaceArea = 0;
		for(size_t i = 0; i < usedRectangles.size(); ++i)
			usedSurfaceArea += usedRectangles[i].width * usedRectangles[i].height;

		return (Real)usedSurfaceArea / (binWidth * binHeight);
	}

	/// Returns the heuristic score value for placing a rectangle of size width*height into freeRect. Does not try to rotate.
	Int GuillotineBinPack::ScoreByHeuristic(Int width, Int height, const BinPackRectangle &freeRect, FreeRectChoiceHeuristic rectChoice)
	{
		switch(rectChoice)
		{
		case RectBestAreaFit: return ScoreBestAreaFit(width, height, freeRect);
		case RectBestShortSideFit: return ScoreBestShortSideFit(width, height, freeRect);
		case RectBestLongSideFit: return ScoreBestLongSideFit(width, height, freeRect);
		case RectWorstAreaFit: return ScoreWorstAreaFit(width, height, freeRect);
		case RectWorstShortSideFit: return ScoreWorstShortSideFit(width, height, freeRect);
		case RectWorstLongSideFit: return ScoreWorstLongSideFit(width, height, freeRect);
		default: assert(false); return std::numeric_limits<Int>::max();
		}
	}

	Int GuillotineBinPack::ScoreBestAreaFit(Int width, Int height, const BinPackRectangle &freeRect)
	{
		return freeRect.width * freeRect.height - width * height;
	}

	Int GuillotineBinPack::ScoreBestShortSideFit(Int width, Int height, const BinPackRectangle &freeRect)
	{
		Int leftoverHoriz = abs(freeRect.width - width);
		Int leftoverVert = abs(freeRect.height - height);
		Int leftover = min(leftoverHoriz, leftoverVert);
		return leftover;
	}

	Int GuillotineBinPack::ScoreBestLongSideFit(Int width, Int height, const BinPackRectangle &freeRect)
	{
		Int leftoverHoriz = abs(freeRect.width - width);
		Int leftoverVert = abs(freeRect.height - height);
		Int leftover = max(leftoverHoriz, leftoverVert);
		return leftover;
	}

	Int GuillotineBinPack::ScoreWorstAreaFit(Int width, Int height, const BinPackRectangle &freeRect)
	{
		return -ScoreBestAreaFit(width, height, freeRect);
	}

	Int GuillotineBinPack::ScoreWorstShortSideFit(Int width, Int height, const BinPackRectangle &freeRect)
	{
		return -ScoreBestShortSideFit(width, height, freeRect);
	}

	Int GuillotineBinPack::ScoreWorstLongSideFit(Int width, Int height, const BinPackRectangle &freeRect)
	{
		return -ScoreBestLongSideFit(width, height, freeRect);
	}

	BinPackRectangle GuillotineBinPack::FindPositionForNewNode(Int width, Int height, FreeRectChoiceHeuristic rectChoice, Int *nodeIndex)
	{
		BinPackRectangle bestNode;
		memset(&bestNode, 0, sizeof(BinPackRectangle));

		Int bestScore = std::numeric_limits<Int>::max();

		/// Try each free rectangle to find the best one for placement.
		for(size_t i = 0; i < freeRectangles.size(); ++i)
		{
			// If this is a perfect fit upright, choose it immediately.
			if (width == freeRectangles[i].width && height == freeRectangles[i].height)
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = width;
				bestNode.height = height;
				bestScore = std::numeric_limits<Int>::min();
				*nodeIndex = i;
				break;
			}
			// If this is a perfect fit sideways, choose it.
			else if (height == freeRectangles[i].width && width == freeRectangles[i].height)
			{
				bestNode.x = freeRectangles[i].x;
				bestNode.y = freeRectangles[i].y;
				bestNode.width = height;
				bestNode.height = width;
				bestScore = std::numeric_limits<Int>::min();
				*nodeIndex = i;
				break;
			}
			// Does the rectangle fit upright?
			else if (width <= freeRectangles[i].width && height <= freeRectangles[i].height)
			{
				Int score = ScoreByHeuristic(width, height, freeRectangles[i], rectChoice);

				if (score < bestScore)
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestScore = score;
					*nodeIndex = i;
				}
			}
			// Does the rectangle fit sideways?
			else if (height <= freeRectangles[i].width && width <= freeRectangles[i].height)
			{
				Int score = ScoreByHeuristic(height, width, freeRectangles[i], rectChoice);

				if (score < bestScore)
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestScore = score;
					*nodeIndex = i;
				}
			}
		}
		return bestNode;
	}

	void GuillotineBinPack::SplitFreeRectByHeuristic(const BinPackRectangle &freeRect, const BinPackRectangle &placedRect, GuillotineSplitHeuristic method)
	{
		// Compute the lengths of the leftover area.
		const Int w = freeRect.width - placedRect.width;
		const Int h = freeRect.height - placedRect.height;

		// Placing placedRect into freeRect results in an L-shaped free area, which must be split into
		// two disjoint rectangles. This can be achieved with by splitting the L-shape using a single line.
		// We have two choices: horizontal or vertical.	

		// Use the given heuristic to decide which choice to make.

		Bool splitHorizontal;
		switch(method)
		{
		case SplitShorterLeftoverAxis:
			// Split along the shorter leftover axis.
			splitHorizontal = (w <= h);
			break;
		case SplitLongerLeftoverAxis:
			// Split along the longer leftover axis.
			splitHorizontal = (w > h);
			break;
		case SplitMinimizeArea:
			// Maximize the larger area == minimize the smaller area.
			// Tries to make the single bigger rectangle.
			splitHorizontal = (placedRect.width * h > w * placedRect.height);
			break;
		case SplitMaximizeArea:
			// Maximize the smaller area == minimize the larger area.
			// Tries to make the rectangles more even-sized.
			splitHorizontal = (placedRect.width * h <= w * placedRect.height);
			break;
		case SplitShorterAxis:
			// Split along the shorter total axis.
			splitHorizontal = (freeRect.width <= freeRect.height);
			break;
		case SplitLongerAxis:
			// Split along the longer total axis.
			splitHorizontal = (freeRect.width > freeRect.height);
			break;
		default:
			splitHorizontal = true;
			assert(false);
		}

		// Perform the actual split.
		SplitFreeRectAlongAxis(freeRect, placedRect, splitHorizontal);
	}

	/// This function will add the two generated rectangles into the freeRectangles array. The caller is expected to
	/// remove the original rectangle from the freeRectangles array after that.
	void GuillotineBinPack::SplitFreeRectAlongAxis(const BinPackRectangle &freeRect, const BinPackRectangle &placedRect, Bool splitHorizontal)
	{
		// Form the two new rectangles.
		BinPackRectangle bottom;
		bottom.x = freeRect.x;
		bottom.y = freeRect.y + placedRect.height;
		bottom.height = freeRect.height - placedRect.height;

		BinPackRectangle right;
		right.x = freeRect.x + placedRect.width;
		right.y = freeRect.y;
		right.width = freeRect.width - placedRect.width;

		if (splitHorizontal)
		{
			bottom.width = freeRect.width;
			right.height = placedRect.height;
		}
		else // Split vertically
		{
			bottom.width = placedRect.width;
			right.height = freeRect.height;
		}

		// Add the new rectangles into the free rectangle pool if they weren't degenerate.
		if (bottom.width > 0 && bottom.height > 0)
			freeRectangles.push_back(bottom);
		if (right.width > 0 && right.height > 0)
			freeRectangles.push_back(right);
	}

	void GuillotineBinPack::MergeFreeList()
	{
		// Do a Theta(n^2) loop to see if any pair of free rectangles could me merged into one.
		// Note that we miss any opportunities to merge three rectangles into one. (should call this function again to detect that)
		for(size_t i = 0; i < freeRectangles.size(); ++i)
			for(size_t j = i+1; j < freeRectangles.size(); ++j)
			{
				if (freeRectangles[i].width == freeRectangles[j].width && freeRectangles[i].x == freeRectangles[j].x)
				{
					if (freeRectangles[i].y == freeRectangles[j].y + freeRectangles[j].height)
					{
						freeRectangles[i].y -= freeRectangles[j].height;
						freeRectangles[i].height += freeRectangles[j].height;
						freeRectangles.erase(freeRectangles.begin() + j);
						--j;
					}
					else if (freeRectangles[i].y + freeRectangles[i].height == freeRectangles[j].y)
					{
						freeRectangles[i].height += freeRectangles[j].height;
						freeRectangles.erase(freeRectangles.begin() + j);
						--j;
					}
				}
				else if (freeRectangles[i].height == freeRectangles[j].height && freeRectangles[i].y == freeRectangles[j].y)
				{
					if (freeRectangles[i].x == freeRectangles[j].x + freeRectangles[j].width)
					{
						freeRectangles[i].x -= freeRectangles[j].width;
						freeRectangles[i].width += freeRectangles[j].width;
						freeRectangles.erase(freeRectangles.begin() + j);
						--j;
					}
					else if (freeRectangles[i].x + freeRectangles[i].width == freeRectangles[j].x)
					{
						freeRectangles[i].width += freeRectangles[j].width;
						freeRectangles.erase(freeRectangles.begin() + j);
						--j;
					}
				}
			}
	}


	//
	// ShelfBinPack
	//

	ShelfBinPack::ShelfBinPack()
		:binWidth(0),
		binHeight(0),
		useWasteMap(false),
		currentY(0),
		usedSurfaceArea(0)
	{
	}

	ShelfBinPack::ShelfBinPack(Int width, Int height, Bool useWasteMap)
	{
		Init(width, height, useWasteMap);
	}

	void ShelfBinPack::Init(Int width, Int height, Bool useWasteMap_)
	{
		useWasteMap = useWasteMap_;
		binWidth = width;
		binHeight = height;

		currentY = 0;
		usedSurfaceArea = 0;

		shelves.clear();
		StartNewShelf(0);

		if (useWasteMap)
		{
			wasteMap.Init(width, height);
			wasteMap.GetFreeRectangles().clear();
		}
	}

	Bool ShelfBinPack::CanStartNewShelf(Int height) const
	{
		return shelves.back().startY + shelves.back().height + height <= binHeight;
	}

	void ShelfBinPack::StartNewShelf(Int startingHeight)
	{
		if (shelves.size() > 0)
		{
			assert(shelves.back().height != 0);
			currentY += shelves.back().height;

			assert(currentY < binHeight);
		}

		Shelf shelf;
		shelf.currentX = 0;
		shelf.height = startingHeight;
		shelf.startY = currentY;

		assert(shelf.startY + shelf.height <= binHeight);
		shelves.push_back(shelf);
	}

	Bool ShelfBinPack::FitsOnShelf(const Shelf &shelf, Int width, Int height, Bool canResize) const
	{
		const Int shelfHeight = canResize ? (binHeight - shelf.startY) : shelf.height;
		if ((shelf.currentX + width <= binWidth && height <= shelfHeight) ||
			(shelf.currentX + height <= binWidth && width <= shelfHeight))
			return true;
		else
			return false;
	}

	void ShelfBinPack::RotateToShelf(const Shelf &shelf, Int &width, Int &height) const
	{	
		// If the width > height and the long edge of the new rectangle fits vertically onto the current shelf,
		// flip it. If the short edge is larger than the current shelf height, store
		// the short edge vertically.
		if ((width > height && width > binWidth - shelf.currentX) ||
			(width > height && width < shelf.height) ||
			(width < height && height > shelf.height && height <= binWidth - shelf.currentX))
			std::swap(width, height);
	}

	void ShelfBinPack::AddToShelf(Shelf &shelf, Int width, Int height, BinPackRectangle &newNode)
	{
		assert(FitsOnShelf(shelf, width, height, true));

		// Swap width and height if the rect fits better that way.
		RotateToShelf(shelf, width, height);

		// Add the rectangle to the shelf.
		newNode.x = shelf.currentX;
		newNode.y = shelf.startY;
		newNode.width = width;
		newNode.height = height;
		shelf.usedRectangles.push_back(newNode);

		// Advance the shelf end position horizontally.
		shelf.currentX += width;
		assert(shelf.currentX <= binWidth);

		// Grow the shelf height.
		shelf.height = max(shelf.height, height);
		assert(shelf.height <= binHeight);

		usedSurfaceArea += width * height;
	}

	BinPackRectangle ShelfBinPack::Insert(Int width, Int height, ShelfChoiceHeuristic method)
	{
		BinPackRectangle newNode;

		// First try to pack this rectangle into the waste map, if it fits.
		if (useWasteMap)
		{
			newNode = wasteMap.Insert(width, height, true, GuillotineBinPack::RectBestShortSideFit, 
				GuillotineBinPack::SplitMaximizeArea);
			if (newNode.height != 0)
			{
				// Track the space we just used.
				usedSurfaceArea += width * height;

				return newNode;
			}
		}

		switch(method)
		{
		case ShelfNextFit:		
			if (FitsOnShelf(shelves.back(), width, height, true))
			{
				AddToShelf(shelves.back(), width, height, newNode);
				return newNode;
			}
			break;
		case ShelfFirstFit:		
			for(size_t i = 0; i < shelves.size(); ++i)
				if (FitsOnShelf(shelves[i], width, height, i == shelves.size()-1))
				{
					AddToShelf(shelves[i], width, height, newNode);
					return newNode;
				}
				break;

		case ShelfBestAreaFit:
			{
				// Best Area Fit rule: Choose the shelf with smallest remaining shelf area.
				Shelf *bestShelf = 0;
				Ulong bestShelfSurfaceArea = (Ulong)-1;
				for(size_t i = 0; i < shelves.size(); ++i)
				{
					// Pre-rotate the rect onto the shelf here already so that the area fit computation
					// is done correctly.
					RotateToShelf(shelves[i], width, height);
					if (FitsOnShelf(shelves[i], width, height, i == shelves.size()-1))
					{
						Ulong surfaceArea = (binWidth - shelves[i].currentX) * shelves[i].height;
						if (surfaceArea < bestShelfSurfaceArea)
						{
							bestShelf = &shelves[i];
							bestShelfSurfaceArea = surfaceArea;
						}
					}
				}

				if (bestShelf)
				{
					AddToShelf(*bestShelf, width, height, newNode);
					return newNode;
				}
			}
			break;

		case ShelfWorstAreaFit:
			{
				// Worst Area Fit rule: Choose the shelf with smallest remaining shelf area.
				Shelf *bestShelf = 0;
				Int bestShelfSurfaceArea = -1;
				for(size_t i = 0; i < shelves.size(); ++i)
				{
					// Pre-rotate the rect onto the shelf here already so that the area fit computation
					// is done correctly.
					RotateToShelf(shelves[i], width, height);
					if (FitsOnShelf(shelves[i], width, height, i == shelves.size()-1))
					{
						Int surfaceArea = (binWidth - shelves[i].currentX) * shelves[i].height;
						if (surfaceArea > bestShelfSurfaceArea)
						{
							bestShelf = &shelves[i];
							bestShelfSurfaceArea = surfaceArea;
						}
					}
				}

				if (bestShelf)
				{
					AddToShelf(*bestShelf, width, height, newNode);
					return newNode;
				}
			}
			break;

		case ShelfBestHeightFit:
			{
				// Best Height Fit rule: Choose the shelf with best-matching height.
				Shelf *bestShelf = 0;
				Int bestShelfHeightDifference = 0x7FFFFFFF;
				for(size_t i = 0; i < shelves.size(); ++i)
				{
					// Pre-rotate the rect onto the shelf here already so that the height fit computation
					// is done correctly.
					RotateToShelf(shelves[i], width, height);
					if (FitsOnShelf(shelves[i], width, height, i == shelves.size()-1))
					{
						Int heightDifference = max(shelves[i].height - height, 0);
						assert(heightDifference >= 0);

						if (heightDifference < bestShelfHeightDifference)
						{
							bestShelf = &shelves[i];
							bestShelfHeightDifference = heightDifference;
						}
					}
				}

				if (bestShelf)
				{
					AddToShelf(*bestShelf, width, height, newNode);
					return newNode;
				}
			}
			break;

		case ShelfBestWidthFit:
			{
				// Best Width Fit rule: Choose the shelf with smallest remaining shelf width.
				Shelf *bestShelf = 0;
				Int bestShelfWidthDifference = 0x7FFFFFFF;
				for(size_t i = 0; i < shelves.size(); ++i)
				{
					// Pre-rotate the rect onto the shelf here already so that the height fit computation
					// is done correctly.
					RotateToShelf(shelves[i], width, height);
					if (FitsOnShelf(shelves[i], width, height, i == shelves.size()-1))
					{
						Int widthDifference = binWidth - shelves[i].currentX - width;
						assert(widthDifference >= 0);

						if (widthDifference < bestShelfWidthDifference)
						{
							bestShelf = &shelves[i];
							bestShelfWidthDifference = widthDifference;
						}
					}
				}

				if (bestShelf)
				{
					AddToShelf(*bestShelf, width, height, newNode);
					return newNode;
				}
			}
			break;

		case ShelfWorstWidthFit:
			{
				// Worst Width Fit rule: Choose the shelf with smallest remaining shelf width.
				Shelf *bestShelf = 0;
				Int bestShelfWidthDifference = -1;
				for(size_t i = 0; i < shelves.size(); ++i)
				{
					// Pre-rotate the rect onto the shelf here already so that the height fit computation
					// is done correctly.
					RotateToShelf(shelves[i], width, height);
					if (FitsOnShelf(shelves[i], width, height, i == shelves.size()-1))
					{
						Int widthDifference = binWidth - shelves[i].currentX - width;
						assert(widthDifference >= 0);

						if (widthDifference > bestShelfWidthDifference)
						{
							bestShelf = &shelves[i];
							bestShelfWidthDifference = widthDifference;
						}
					}
				}

				if (bestShelf)
				{
					AddToShelf(*bestShelf, width, height, newNode);
					return newNode;
				}
			}
			break;

		}

		// The rectangle did not fit on any of the shelves. Open a new shelf.

		// Flip the rectangle so that the long side is horizontal.
		if (width < height && height <= binWidth)
			std::swap(width, height);

		if (CanStartNewShelf(height))
		{
			if (useWasteMap)
				MoveShelfToWasteMap(shelves.back());
			StartNewShelf(height);
			assert(FitsOnShelf(shelves.back(), width, height, true));
			AddToShelf(shelves.back(), width, height, newNode);
			return newNode;
		}
		/*
		///\todo This is problematic: If we couldn't start a new shelf - should we give up
		///      and move all the remaining space of the bin for the waste map to track,
		///      or should we just wait if the next rectangle would fit better? For now,
		///      don't add the leftover space to the waste map. 
		else if (useWasteMap)
		{
		assert(binHeight - shelves.back().startY >= shelves.back().height);
		shelves.back().height = binHeight - shelves.back().startY;
		if (shelves.back().height > 0)
		MoveShelfToWasteMap(shelves.back());

		// Try to pack the rectangle again to the waste map.
		GuillotineBinPack::Node node = wasteMap.Insert(width, height, true, 1, 3);
		if (node.height != 0)
		{
		newNode.x = node.x;
		newNode.y = node.y;
		newNode.width = node.width;
		newNode.height = node.height;
		return newNode;
		}
		}
		*/

		// The rectangle didn't fit.
		memset(&newNode, 0, sizeof(BinPackRectangle));
		return newNode;
	}

	void ShelfBinPack::MoveShelfToWasteMap(Shelf &shelf)
	{
		BinPackRectangles &freeRects = wasteMap.GetFreeRectangles();

		// Add the gaps between each rect top and shelf ceiling to the waste map.
		for(size_t i = 0; i < shelf.usedRectangles.size(); ++i)
		{
			const BinPackRectangle &r = shelf.usedRectangles[i];
			BinPackRectangle newNode;
			newNode.x = r.x;
			newNode.y = r.y + r.height;
			newNode.width = r.width;
			newNode.height = shelf.height - r.height;
			if (newNode.height > 0)
				freeRects.push_back(newNode);		
		}
		shelf.usedRectangles.clear();

		// Add the space after the shelf end (right side of the last rect) and the shelf right side. 
		BinPackRectangle newNode;
		newNode.x = shelf.currentX;
		newNode.y = shelf.startY;
		newNode.width = binWidth - shelf.currentX;
		newNode.height = shelf.height;
		if (newNode.width > 0)
			freeRects.push_back(newNode);

		// This shelf is DONE.
		shelf.currentX = binWidth;

		// Perform a rectangle merge step.
		wasteMap.MergeFreeList();
	}

	/// Computes the ratio of used surface area to the bin area.
	Real ShelfBinPack::Occupancy() const
	{
		return (Real)usedSurfaceArea / (binWidth * binHeight);
	}

	//
	// MaxRectsBinPack
	//

	MaxRectsBinPack::MaxRectsBinPack()
		:binWidth(0),
		binHeight(0)
	{
	}

	MaxRectsBinPack::MaxRectsBinPack(Int width, Int height)
	{
		Init(width, height);
	}

	void MaxRectsBinPack::Init(Int width, Int height)
	{
		binWidth = width;
		binHeight = height;

		BinPackRectangle n;
		n.x = 0;
		n.y = 0;
		n.width = width;
		n.height = height;

		usedRectangles.clear();

		freeRectangles.clear();
		freeRectangles.push_back(n);
	}

	BinPackRectangle MaxRectsBinPack::Insert(Int width, Int height, FreeRectChoiceHeuristic method)
	{
		BinPackRectangle newNode;
		Int score1; // Unused in this function. We don't need to know the score after finding the position.
		Int score2;
		switch(method)
		{
		case RectBestShortSideFit: newNode = FindPositionForNewNodeBestShortSideFit(width, height, score1, score2); break;
		case RectBottomLeftRule: newNode = FindPositionForNewNodeBottomLeft(width, height, score1, score2); break;
		case RectContactPointRule: newNode = FindPositionForNewNodeContactPoint(width, height, score1); break;
		case RectBestLongSideFit: newNode = FindPositionForNewNodeBestLongSideFit(width, height, score2, score1); break;
		case RectBestAreaFit: newNode = FindPositionForNewNodeBestAreaFit(width, height, score1, score2); break;
		}

		if (newNode.height == 0)
			return newNode;

		size_t numRectanglesToProcess = freeRectangles.size();
		for(size_t i = 0; i < numRectanglesToProcess; ++i)
		{
			if (SplitFreeNode(freeRectangles[i], newNode))
			{
				freeRectangles.erase(freeRectangles.begin() + i);
				--i;
				--numRectanglesToProcess;
			}
		}

		PruneFreeList();

		usedRectangles.push_back(newNode);
		return newNode;
	}

	void MaxRectsBinPack::Insert(BinPackRectangles &rects, BinPackRectangles &dst, FreeRectChoiceHeuristic method)
	{
		dst.clear();

		while(rects.size() > 0)
		{
			Int bestScore1 = std::numeric_limits<Int>::max();
			Int bestScore2 = std::numeric_limits<Int>::max();
			Int bestRectIndex = -1;
			BinPackRectangle bestNode;

			for(size_t i = 0; i < rects.size(); ++i)
			{
				Int score1;
				Int score2;
				BinPackRectangle newNode = ScoreRect(rects[i].width, rects[i].height, method, score1, score2);

				if (score1 < bestScore1 || (score1 == bestScore1 && score2 < bestScore2))
				{
					bestScore1 = score1;
					bestScore2 = score2;
					bestNode = newNode;
					bestRectIndex = i;
				}
			}

			if (bestRectIndex == -1)
				return;

			PlaceRect(bestNode);
			rects.erase(rects.begin() + bestRectIndex);
		}
	}

	void MaxRectsBinPack::PlaceRect(const BinPackRectangle &node)
	{
		size_t numRectanglesToProcess = freeRectangles.size();
		for(size_t i = 0; i < numRectanglesToProcess; ++i)
		{
			if (SplitFreeNode(freeRectangles[i], node))
			{
				freeRectangles.erase(freeRectangles.begin() + i);
				--i;
				--numRectanglesToProcess;
			}
		}

		PruneFreeList();

		usedRectangles.push_back(node);
		//		dst.push_back(bestNode); ///\todo Refactor so that this compiles.
	}

	BinPackRectangle MaxRectsBinPack::ScoreRect(Int width, Int height, FreeRectChoiceHeuristic method, Int &score1, Int &score2) const
	{
		BinPackRectangle newNode;
		score1 = std::numeric_limits<Int>::max();
		score2 = std::numeric_limits<Int>::max();
		switch(method)
		{
		case RectBestShortSideFit: newNode = FindPositionForNewNodeBestShortSideFit(width, height, score1, score2); break;
		case RectBottomLeftRule: newNode = FindPositionForNewNodeBottomLeft(width, height, score1, score2); break;
		case RectContactPointRule: newNode = FindPositionForNewNodeContactPoint(width, height, score1); 
			score1 = -score1; // Reverse since we are minimizing, but for contact point score bigger is better.
			break;
		case RectBestLongSideFit: newNode = FindPositionForNewNodeBestLongSideFit(width, height, score2, score1); break;
		case RectBestAreaFit: newNode = FindPositionForNewNodeBestAreaFit(width, height, score1, score2); break;
		}

		// Cannot fit the current rectangle.
		if (newNode.height == 0)
		{
			score1 = std::numeric_limits<Int>::max();
			score2 = std::numeric_limits<Int>::max();
		}

		return newNode;
	}

	/// Computes the ratio of used surface area.
	Real MaxRectsBinPack::Occupancy() const
	{
		Ulong usedSurfaceArea = 0;
		for(size_t i = 0; i < usedRectangles.size(); ++i)
			usedSurfaceArea += usedRectangles[i].width * usedRectangles[i].height;

		return (Real)usedSurfaceArea / (binWidth * binHeight);
	}

	BinPackRectangle MaxRectsBinPack::FindPositionForNewNodeBottomLeft(Int width, Int height, Int &bestY, Int &bestX) const
	{
		BinPackRectangle bestNode;
		memset(&bestNode, 0, sizeof(BinPackRectangle));

		bestY = std::numeric_limits<Int>::max();

		for(size_t i = 0; i < freeRectangles.size(); ++i)
		{
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				Int topSideY = freeRectangles[i].y + height;
				if (topSideY < bestY || (topSideY == bestY && freeRectangles[i].x < bestX))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestY = topSideY;
					bestX = freeRectangles[i].x;
				}
			}
			if (freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				Int topSideY = freeRectangles[i].y + width;
				if (topSideY < bestY || (topSideY == bestY && freeRectangles[i].x < bestX))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestY = topSideY;
					bestX = freeRectangles[i].x;
				}
			}
		}
		return bestNode;
	}

	BinPackRectangle MaxRectsBinPack::FindPositionForNewNodeBestShortSideFit(Int width, Int height, 
		Int &bestShortSideFit, Int &bestLongSideFit) const
	{
		BinPackRectangle bestNode;
		memset(&bestNode, 0, sizeof(BinPackRectangle));

		bestShortSideFit = std::numeric_limits<Int>::max();

		for(size_t i = 0; i < freeRectangles.size(); ++i)
		{
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				Int leftoverHoriz = abs(freeRectangles[i].width - width);
				Int leftoverVert = abs(freeRectangles[i].height - height);
				Int shortSideFit = min(leftoverHoriz, leftoverVert);
				Int longSideFit = max(leftoverHoriz, leftoverVert);

				if (shortSideFit < bestShortSideFit || (shortSideFit == bestShortSideFit && longSideFit < bestLongSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestShortSideFit = shortSideFit;
					bestLongSideFit = longSideFit;
				}
			}

			if (freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				Int flippedLeftoverHoriz = abs(freeRectangles[i].width - height);
				Int flippedLeftoverVert = abs(freeRectangles[i].height - width);
				Int flippedShortSideFit = min(flippedLeftoverHoriz, flippedLeftoverVert);
				Int flippedLongSideFit = max(flippedLeftoverHoriz, flippedLeftoverVert);

				if (flippedShortSideFit < bestShortSideFit || (flippedShortSideFit == bestShortSideFit && flippedLongSideFit < bestLongSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestShortSideFit = flippedShortSideFit;
					bestLongSideFit = flippedLongSideFit;
				}
			}
		}
		return bestNode;
	}

	BinPackRectangle MaxRectsBinPack::FindPositionForNewNodeBestLongSideFit(Int width, Int height, 
		Int &bestShortSideFit, Int &bestLongSideFit) const
	{
		BinPackRectangle bestNode;
		memset(&bestNode, 0, sizeof(BinPackRectangle));

		bestLongSideFit = std::numeric_limits<Int>::max();

		for(size_t i = 0; i < freeRectangles.size(); ++i)
		{
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				Int leftoverHoriz = abs(freeRectangles[i].width - width);
				Int leftoverVert = abs(freeRectangles[i].height - height);
				Int shortSideFit = min(leftoverHoriz, leftoverVert);
				Int longSideFit = max(leftoverHoriz, leftoverVert);

				if (longSideFit < bestLongSideFit || (longSideFit == bestLongSideFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestShortSideFit = shortSideFit;
					bestLongSideFit = longSideFit;
				}
			}

			if (freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				Int leftoverHoriz = abs(freeRectangles[i].width - height);
				Int leftoverVert = abs(freeRectangles[i].height - width);
				Int shortSideFit = min(leftoverHoriz, leftoverVert);
				Int longSideFit = max(leftoverHoriz, leftoverVert);

				if (longSideFit < bestLongSideFit || (longSideFit == bestLongSideFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestShortSideFit = shortSideFit;
					bestLongSideFit = longSideFit;
				}
			}
		}
		return bestNode;
	}

	BinPackRectangle MaxRectsBinPack::FindPositionForNewNodeBestAreaFit(Int width, Int height, 
		Int &bestAreaFit, Int &bestShortSideFit) const
	{
		BinPackRectangle bestNode;
		memset(&bestNode, 0, sizeof(BinPackRectangle));

		bestAreaFit = std::numeric_limits<Int>::max();

		for(size_t i = 0; i < freeRectangles.size(); ++i)
		{
			Int areaFit = freeRectangles[i].width * freeRectangles[i].height - width * height;

			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				Int leftoverHoriz = abs(freeRectangles[i].width - width);
				Int leftoverVert = abs(freeRectangles[i].height - height);
				Int shortSideFit = min(leftoverHoriz, leftoverVert);

				if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestShortSideFit = shortSideFit;
					bestAreaFit = areaFit;
				}
			}

			if (freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				Int leftoverHoriz = abs(freeRectangles[i].width - height);
				Int leftoverVert = abs(freeRectangles[i].height - width);
				Int shortSideFit = min(leftoverHoriz, leftoverVert);

				if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestShortSideFit = shortSideFit;
					bestAreaFit = areaFit;
				}
			}
		}
		return bestNode;
	}

	/// Returns 0 if the two intervals i1 and i2 are disjoint, or the length of their overlap otherwise.
	Int CommonIntervalLength(Int i1start, Int i1end, Int i2start, Int i2end)
	{
		if (i1end < i2start || i2end < i1start)
			return 0;
		return min(i1end, i2end) - max(i1start, i2start);
	}

	Int MaxRectsBinPack::ContactPointScoreNode(Int x, Int y, Int width, Int height) const
	{
		Int score = 0;

		if (x == 0 || x + width == binWidth)
			score += height;
		if (y == 0 || y + height == binHeight)
			score += width;

		for(size_t i = 0; i < usedRectangles.size(); ++i)
		{
			if (usedRectangles[i].x == x + width || usedRectangles[i].x + usedRectangles[i].width == x)
				score += CommonIntervalLength(usedRectangles[i].y, usedRectangles[i].y + usedRectangles[i].height, y, y + height);
			if (usedRectangles[i].y == y + height || usedRectangles[i].y + usedRectangles[i].height == y)
				score += CommonIntervalLength(usedRectangles[i].x, usedRectangles[i].x + usedRectangles[i].width, x, x + width);
		}
		return score;
	}

	BinPackRectangle MaxRectsBinPack::FindPositionForNewNodeContactPoint(Int width, Int height, Int &bestContactScore) const
	{
		BinPackRectangle bestNode;
		memset(&bestNode, 0, sizeof(BinPackRectangle));

		bestContactScore = -1;

		for(size_t i = 0; i < freeRectangles.size(); ++i)
		{
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				Int score = ContactPointScoreNode(freeRectangles[i].x, freeRectangles[i].y, width, height);
				if (score > bestContactScore)
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestContactScore = score;
				}
			}
			if (freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				Int score = ContactPointScoreNode(freeRectangles[i].x, freeRectangles[i].y, width, height);
				if (score > bestContactScore)
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestContactScore = score;
				}
			}
		}
		return bestNode;
	}

	Bool MaxRectsBinPack::SplitFreeNode(BinPackRectangle freeNode, const BinPackRectangle &usedNode)
	{
		// Test with SAT if the rectangles even intersect.
		if (usedNode.x >= freeNode.x + freeNode.width || usedNode.x + usedNode.width <= freeNode.x ||
			usedNode.y >= freeNode.y + freeNode.height || usedNode.y + usedNode.height <= freeNode.y)
			return false;

		if (usedNode.x < freeNode.x + freeNode.width && usedNode.x + usedNode.width > freeNode.x)
		{
			// New node at the top side of the used node.
			if (usedNode.y > freeNode.y && usedNode.y < freeNode.y + freeNode.height)
			{
				BinPackRectangle newNode = freeNode;
				newNode.height = usedNode.y - newNode.y;
				freeRectangles.push_back(newNode);
			}

			// New node at the bottom side of the used node.
			if (usedNode.y + usedNode.height < freeNode.y + freeNode.height)
			{
				BinPackRectangle newNode = freeNode;
				newNode.y = usedNode.y + usedNode.height;
				newNode.height = freeNode.y + freeNode.height - (usedNode.y + usedNode.height);
				freeRectangles.push_back(newNode);
			}
		}

		if (usedNode.y < freeNode.y + freeNode.height && usedNode.y + usedNode.height > freeNode.y)
		{
			// New node at the left side of the used node.
			if (usedNode.x > freeNode.x && usedNode.x < freeNode.x + freeNode.width)
			{
				BinPackRectangle newNode = freeNode;
				newNode.width = usedNode.x - newNode.x;
				freeRectangles.push_back(newNode);
			}

			// New node at the right side of the used node.
			if (usedNode.x + usedNode.width < freeNode.x + freeNode.width)
			{
				BinPackRectangle newNode = freeNode;
				newNode.x = usedNode.x + usedNode.width;
				newNode.width = freeNode.x + freeNode.width - (usedNode.x + usedNode.width);
				freeRectangles.push_back(newNode);
			}
		}

		return true;
	}

	void MaxRectsBinPack::PruneFreeList()
	{
		/* 
		///  Would be nice to do something like this, to avoid a Theta(n^2) loop through each pair.
		///  But unfortunately it doesn't quite cut it, since we also want to detect containment. 
		///  Perhaps there's another way to do this faster than Theta(n^2).

		if (freeRectangles.size() > 0)
		clb::sort::QuickSort(&freeRectangles[0], freeRectangles.size(), NodeSortCmp);

		for(size_t i = 0; i < freeRectangles.size()-1; ++i)
		if (freeRectangles[i].x == freeRectangles[i+1].x &&
		freeRectangles[i].y == freeRectangles[i+1].y &&
		freeRectangles[i].width == freeRectangles[i+1].width &&
		freeRectangles[i].height == freeRectangles[i+1].height)
		{
		freeRectangles.erase(freeRectangles.begin() + i);
		--i;
		}
		*/

		/// Go through each pair and remove any rectangle that is redundant.
		for(size_t i = 0; i < freeRectangles.size(); ++i)
			for(size_t j = i+1; j < freeRectangles.size(); ++j)
			{
				if (IsContainedIn(freeRectangles[i], freeRectangles[j]))
				{
					freeRectangles.erase(freeRectangles.begin()+i);
					--i;
					break;
				}
				if (IsContainedIn(freeRectangles[j], freeRectangles[i]))
				{
					freeRectangles.erase(freeRectangles.begin()+j);
					--j;
				}
			}
	}


	//
	// SkylineBinPack
	//

	SkylineBinPack::SkylineBinPack()
		:binWidth(0),
		binHeight(0)
	{
	}

	SkylineBinPack::SkylineBinPack(Int width, Int height, Bool useWasteMap)
	{
		Init(width, height, useWasteMap);
	}

	void SkylineBinPack::Init(Int width, Int height, Bool useWasteMap_)
	{
		binWidth = width;
		binHeight = height;

		useWasteMap = useWasteMap_;

		usedSurfaceArea = 0;
		skyLine.clear();
		SkylineNode node;
		node.x = 0;
		node.y = 0;
		node.width = binWidth;
		skyLine.push_back(node);

		if (useWasteMap)
		{
			wasteMap.Init(width, height);
			wasteMap.GetFreeRectangles().clear();
		}
	}

	void SkylineBinPack::Insert(BinPackRectangles &rects, BinPackRectangles &dst, LevelChoiceHeuristic method)
	{
		dst.clear();

		while(rects.size() > 0)
		{
			BinPackRectangle bestNode;
			Int bestScore1 = std::numeric_limits<Int>::max();
			Int bestScore2 = std::numeric_limits<Int>::max();
			Int bestSkylineIndex = -1;
			Int bestRectIndex = -1;
			for(size_t i = 0; i < rects.size(); ++i)
			{
				BinPackRectangle newNode;
				Int score1;
				Int score2;
				Int index;
				switch(method)
				{
				case LevelBottomLeft:
					newNode = FindPositionForNewNodeBottomLeft(rects[i].width, rects[i].height, score1, score2, index);
					break;
				case LevelMinWasteFit:
					newNode = FindPositionForNewNodeMinWaste(rects[i].width, rects[i].height, score2, score1, index);
					break;
				default: assert(false); break;
				}
				if (newNode.height != 0)
				{
					if (score1 < bestScore1 || (score1 == bestScore1 && score2 < bestScore2))
					{
						bestNode = newNode;
						bestScore1 = score1;
						bestScore2 = score2;
						bestSkylineIndex = index;
						bestRectIndex = i;
					}
				}
			}

			if (bestRectIndex == -1)
				return;

			// Perform the actual packing.

			AddSkylineLevel(bestSkylineIndex, bestNode);
			usedSurfaceArea += rects[bestRectIndex].width * rects[bestRectIndex].height;
			rects.erase(rects.begin() + bestRectIndex);
			dst.push_back(bestNode);
		}
	}

	BinPackRectangle SkylineBinPack::Insert(Int width, Int height, LevelChoiceHeuristic method)
	{
		// First try to pack this rectangle into the waste map, if it fits.
		BinPackRectangle node = wasteMap.Insert(width, height, true, GuillotineBinPack::RectBestShortSideFit, GuillotineBinPack::SplitMaximizeArea);


		if (node.height != 0)
		{
			BinPackRectangle newNode;
			newNode.x = node.x;
			newNode.y = node.y;
			newNode.width = node.width;
			newNode.height = node.height;
			usedSurfaceArea += width * height;

			return newNode;
		}

		switch(method)
		{
		case LevelBottomLeft: return InsertBottomLeft(width, height);
		case LevelMinWasteFit: return InsertMinWaste(width, height);
		default: assert(false); return node;
		}
	}

	Bool SkylineBinPack::RectangleFits(Int skylineNodeIndex, Int width, Int height, Int &y) const
	{
		Int x = skyLine[skylineNodeIndex].x;
		if (x + width > binWidth)
			return false;
		Int widthLeft = width;
		Int i = skylineNodeIndex;
		y = skyLine[skylineNodeIndex].y;
		while(widthLeft > 0)
		{
			y = max(y, skyLine[i].y);
			if (y + height > binHeight)
				return false;
			widthLeft -= skyLine[i].width;
			++i;
			assert(i < (Int)skyLine.size() || widthLeft <= 0);
		}
		return true;
	}

	Int SkylineBinPack::ComputeWastedArea(Int skylineNodeIndex, Int width, Int height, Int y) const
	{
		Int wastedArea = 0;
		const Int rectLeft = skyLine[skylineNodeIndex].x;
		const Int rectRight = rectLeft + width;
		for(; skylineNodeIndex < (Int)skyLine.size() && skyLine[skylineNodeIndex].x < rectRight; ++skylineNodeIndex)
		{
			if (skyLine[skylineNodeIndex].x >= rectRight || skyLine[skylineNodeIndex].x + skyLine[skylineNodeIndex].width <= rectLeft)
				break;

			Int leftSide = skyLine[skylineNodeIndex].x;
			Int rightSide = min(rectRight, leftSide + skyLine[skylineNodeIndex].width);
			assert(y >= skyLine[skylineNodeIndex].y);
			wastedArea += (rightSide - leftSide) * (y - skyLine[skylineNodeIndex].y);
		}
		return wastedArea;
	}

	Bool SkylineBinPack::RectangleFits(Int skylineNodeIndex, Int width, Int height, Int &y, Int &wastedArea) const
	{
		Bool fits = RectangleFits(skylineNodeIndex, width, height, y);
		if (fits)
			wastedArea = ComputeWastedArea(skylineNodeIndex, width, height, y);

		return fits;
	}

	void SkylineBinPack::AddWasteMapArea(Int skylineNodeIndex, Int width, Int height, Int y)
	{
		Int wastedArea = 0;
		const Int rectLeft = skyLine[skylineNodeIndex].x;
		const Int rectRight = rectLeft + width;
		for(; skylineNodeIndex < (Int)skyLine.size() && skyLine[skylineNodeIndex].x < rectRight; ++skylineNodeIndex)
		{
			if (skyLine[skylineNodeIndex].x >= rectRight || skyLine[skylineNodeIndex].x + skyLine[skylineNodeIndex].width <= rectLeft)
				break;

			Int leftSide = skyLine[skylineNodeIndex].x;
			Int rightSide = min(rectRight, leftSide + skyLine[skylineNodeIndex].width);
			assert(y >= skyLine[skylineNodeIndex].y);

			BinPackRectangle waste;
			waste.x = leftSide;
			waste.y = skyLine[skylineNodeIndex].y;
			waste.width = rightSide - leftSide;
			waste.height = y - skyLine[skylineNodeIndex].y;

			wasteMap.GetFreeRectangles().push_back(waste);
		}
	}

	void SkylineBinPack::AddSkylineLevel(Int skylineNodeIndex, const BinPackRectangle &rect)
	{
		// First track all wasted areas and mark them into the waste map if we're using one.
		if (useWasteMap)
			AddWasteMapArea(skylineNodeIndex, rect.width, rect.height, rect.y);

		SkylineNode newNode;
		newNode.x = rect.x;
		newNode.y = rect.y + rect.height;
		newNode.width = rect.width;
		skyLine.insert(skyLine.begin() + skylineNodeIndex, newNode);

		assert(newNode.x + newNode.width <= binWidth);
		assert(newNode.y <= binHeight);

		for(size_t i = skylineNodeIndex+1; i < skyLine.size(); ++i)
		{
			assert(skyLine[i-1].x <= skyLine[i].x);

			if (skyLine[i].x < skyLine[i-1].x + skyLine[i-1].width)
			{
				Int shrink = skyLine[i-1].x + skyLine[i-1].width - skyLine[i].x;

				skyLine[i].x += shrink;
				skyLine[i].width -= shrink;

				if (skyLine[i].width <= 0)
				{
					skyLine.erase(skyLine.begin() + i);
					--i;
				}
				else
					break;
			}
			else
				break;
		}
		MergeSkylines();
	}

	void SkylineBinPack::MergeSkylines()
	{
		for(size_t i = 0; i < skyLine.size()-1; ++i)
			if (skyLine[i].y == skyLine[i+1].y)
			{
				skyLine[i].width += skyLine[i+1].width;
				skyLine.erase(skyLine.begin() + (i+1));
				--i;
			}
	}

	BinPackRectangle SkylineBinPack::InsertBottomLeft(Int width, Int height)
	{
		Int bestHeight;
		Int bestWidth;
		Int bestIndex;
		BinPackRectangle newNode = FindPositionForNewNodeBottomLeft(width, height, bestHeight, bestWidth, bestIndex);

		if (bestIndex != -1)
		{
			// Perform the actual packing.
			AddSkylineLevel(bestIndex, newNode);

			usedSurfaceArea += width * height;
		}
		else
			memset(&newNode, 0, sizeof(BinPackRectangle));

		return newNode;
	}

	BinPackRectangle SkylineBinPack::FindPositionForNewNodeBottomLeft(Int width, Int height, Int &bestHeight, Int &bestWidth, Int &bestIndex) const
	{
		bestHeight = std::numeric_limits<Int>::max();
		bestIndex = -1;
		// Used to break ties if there are nodes at the same level. Then pick the narrowest one.
		bestWidth = std::numeric_limits<Int>::max();
		BinPackRectangle newNode;
		memset(&newNode, 0, sizeof(newNode));
		for(size_t i = 0; i < skyLine.size(); ++i)
		{
			Int y;
			if (RectangleFits(i, width, height, y))
			{
				if (y + height < bestHeight || (y + height == bestHeight && skyLine[i].width < bestWidth))
				{
					bestHeight = y + height;
					bestIndex = i;
					bestWidth = skyLine[i].width;
					newNode.x = skyLine[i].x;
					newNode.y = y;
					newNode.width = width;
					newNode.height = height;
				}
			}
			if (RectangleFits(i, height, width, y))
			{
				if (y + width < bestHeight || (y + width == bestHeight && skyLine[i].width < bestWidth))
				{
					bestHeight = y + width;
					bestIndex = i;
					bestWidth = skyLine[i].width;
					newNode.x = skyLine[i].x;
					newNode.y = y;
					newNode.width = height;
					newNode.height = width;
				}
			}
		}

		return newNode;
	}

	BinPackRectangle SkylineBinPack::InsertMinWaste(Int width, Int height)
	{
		Int bestHeight;
		Int bestWastedArea;
		Int bestIndex;
		BinPackRectangle newNode = FindPositionForNewNodeMinWaste(width, height, bestHeight, bestWastedArea, bestIndex);

		if (bestIndex != -1)
		{
			// Perform the actual packing.
			AddSkylineLevel(bestIndex, newNode);

			usedSurfaceArea += width * height;
		}
		else
			memset(&newNode, 0, sizeof(newNode));

		return newNode;
	}

	BinPackRectangle SkylineBinPack::FindPositionForNewNodeMinWaste(Int width, Int height, Int &bestHeight, Int &bestWastedArea, Int &bestIndex) const
	{
		bestHeight = std::numeric_limits<Int>::max();
		bestWastedArea = std::numeric_limits<Int>::max();
		bestIndex = -1;
		BinPackRectangle newNode;
		memset(&newNode, 0, sizeof(newNode));
		for(size_t i = 0; i < skyLine.size(); ++i)
		{
			Int y;
			Int wastedArea;

			if (RectangleFits(i, width, height, y, wastedArea))
			{
				if (wastedArea < bestWastedArea || (wastedArea == bestWastedArea && y + height < bestHeight))
				{
					bestHeight = y + height;
					bestWastedArea = wastedArea;
					bestIndex = i;
					newNode.x = skyLine[i].x;
					newNode.y = y;
					newNode.width = width;
					newNode.height = height;
				}
			}
			if (RectangleFits(i, height, width, y, wastedArea))
			{
				if (wastedArea < bestWastedArea || (wastedArea == bestWastedArea && y + width < bestHeight))
				{
					bestHeight = y + width;
					bestWastedArea = wastedArea;
					bestIndex = i;
					newNode.x = skyLine[i].x;
					newNode.y = y;
					newNode.width = height;
					newNode.height = width;
				}
			}
		}

		return newNode;
	}

	/// Computes the ratio of used surface area.
	Real SkylineBinPack::Occupancy() const
	{
		return (Real)usedSurfaceArea / (binWidth * binHeight);
	}

	MetaBinPack::MetaBinPack()
	{
	}

	Bool MetaBinPack::pack(MetaRectangles &rectangles, Int& width, Int& height)
	{
		width = 16;
		height = 16;

		Int const max_runs = 15;
		Int const max_algorithms = 4;

		Int current_run = 0;
		Bool fits_on_algorithm = false;

		while(!fits_on_algorithm && current_run < max_runs)
		{
			for (int algo = 0; (algo != max_algorithms) && !fits_on_algorithm; algo++)
			{
				fits_on_algorithm = true;

				if (algo == 0)
				{
					m_shelf.Init(width, height, true);
					for (MetaRectangles::iterator r = rectangles.begin(); (r != rectangles.end()) && fits_on_algorithm; ++r)
					{
						BinPackRectangle output = m_shelf.Insert(r->originalWidth, r->originalHeight, ShelfBinPack::ShelfBestAreaFit);
						r->width = output.width;
						r->height = output.height;
						r->x = output.x;
						r->y = output.y;

						if ((output.width == 0) || (output.height == 0))
							fits_on_algorithm = false;
					}
				}
				else if (algo == 1)
				{
					m_guillotine.Init(width, height);
					for (MetaRectangles::iterator r = rectangles.begin(); (r != rectangles.end()) && fits_on_algorithm; ++r)
					{
						BinPackRectangle output = m_guillotine.Insert(r->originalWidth, r->originalHeight, true, GuillotineBinPack::RectBestShortSideFit, GuillotineBinPack::SplitMinimizeArea);
						r->width = output.width;
						r->height = output.height;
						r->x = output.x;
						r->y = output.y;

						if ((output.width == 0) || (output.height == 0))
							fits_on_algorithm = false;
					}
				}
				else if (algo == 2)
				{
					m_skyline.Init(width, height, true);
					for (MetaRectangles::iterator r = rectangles.begin(); (r != rectangles.end()) && fits_on_algorithm; ++r)
					{
						BinPackRectangle output = m_skyline.Insert(r->originalWidth, r->originalHeight, SkylineBinPack::LevelBottomLeft);
						r->width = output.width;
						r->height = output.height;
						r->x = output.x;
						r->y = output.y;

						if ((output.width == 0) || (output.height == 0))
							fits_on_algorithm = false;
					}
				}
				else if (algo == 3)
				{
					m_max_rects.Init(width, height);
					for (MetaRectangles::iterator r = rectangles.begin(); (r != rectangles.end()) && fits_on_algorithm; ++r)
					{
						BinPackRectangle output = m_max_rects.Insert(r->originalWidth, r->originalHeight, MaxRectsBinPack::RectBestShortSideFit);
						r->width = output.width;
						r->height = output.height;
						r->x = output.x;
						r->y = output.y;

						if ((output.width == 0) || (output.height == 0))
							fits_on_algorithm = false;
					}
				}
			}

			if (!fits_on_algorithm)
			{
				width *= 2;
				height *= 2;
				current_run++;
			}
		}


		if (fits_on_algorithm)
		{
			for (MetaRectangles::iterator r = rectangles.begin(); r != rectangles.end(); ++r)
			{
				if ((r->originalHeight != r->height) || (r->originalWidth != r->width))
				{
					r->rotated = true;
				}
			}
		}
		else
		{
			width = 0;
			height = 0;
		}
		

		return fits_on_algorithm;
	}

} // namespace rengine
