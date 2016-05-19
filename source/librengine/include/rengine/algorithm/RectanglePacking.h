// __!!rengine_copyright!!__ //

#ifndef __RENGINE_RECTANGLE_PACKING_H__
#define __RENGINE_RECTANGLE_PACKING_H__

#include <rengine/lang/Lang.h>

#include <vector>

namespace rengine
{
	struct BinPackRectangle
	{
		BinPackRectangle()
		{
			x = 0;
			y = 0;
			width = 0;
			height = 0;
		}

		Int x;
		Int y;
		Int width;
		Int height;
	};
	typedef std::vector<BinPackRectangle> BinPackRectangles;

	/** GuillotineBinPack implements different variants of bin packer algorithms that use the GUILLOTINE data structure
	to keep track of the free space of the bin where rectangles may be placed. */
	class GuillotineBinPack
	{
	public:
		/// The initial bin size will be (0,0). Call Init to set the bin size.
		GuillotineBinPack();

		/// Initializes a new bin of the given size.
		GuillotineBinPack(Int width, Int height);

		/// (Re)initializes the packer to an empty bin of width x height units. Call whenever
		/// you need to restart with a new bin.
		void Init(Int width, Int height);

		/// Specifies the different choice heuristics that can be used when deciding which of the free subrectangles
		/// to place the to-be-packed rectangle into.
		enum FreeRectChoiceHeuristic
		{
			RectBestAreaFit, ///< -BAF
			RectBestShortSideFit, ///< -BSSF
			RectBestLongSideFit, ///< -BLSF
			RectWorstAreaFit, ///< -WAF
			RectWorstShortSideFit, ///< -WSSF
			RectWorstLongSideFit ///< -WLSF
		};

		/// Specifies the different choice heuristics that can be used when the packer needs to decide whether to
		/// subdivide the remaining free space in horizontal or vertical direction.
		enum GuillotineSplitHeuristic
		{
			SplitShorterLeftoverAxis, ///< -SLAS
			SplitLongerLeftoverAxis, ///< -LLAS
			SplitMinimizeArea, ///< -MINAS, Try to make a single big rectangle at the expense of making the other small.
			SplitMaximizeArea, ///< -MAXAS, Try to make both remaining rectangles as even-sized as possible.
			SplitShorterAxis, ///< -SAS
			SplitLongerAxis ///< -LAS
		};

		/// Inserts a single rectangle into the bin. The packer might rotate the rectangle, in which case the returned
		/// struct will have the width and height values swapped.
		/// @param merge If true, performs free Rectangle Merge procedure after packing the new rectangle. This procedure
		///		tries to defragment the list of disjoint free rectangles to improve packing performance, but also takes up 
		///		some extra time.
		/// @param rectChoice The free rectangle choice heuristic rule to use.
		/// @param splitMethod The free rectangle split heuristic rule to use.
		BinPackRectangle Insert(Int width, Int height, Bool merge, FreeRectChoiceHeuristic rectChoice, GuillotineSplitHeuristic splitMethod);

		/// Inserts a list of rectangles into the bin.
		/// @param rects The list of rectangles to add. This list will be destroyed in the packing process.
		/// @param dst The outputted list of rectangles. Note that the indices will not correspond to the input indices.
		/// @param merge If true, performs Rectangle Merge operations during the packing process.
		/// @param rectChoice The free rectangle choice heuristic rule to use.
		/// @param splitMethod The free rectangle split heuristic rule to use.
		void Insert(BinPackRectangles &rects, BinPackRectangles &dst, Bool merge, FreeRectChoiceHeuristic rectChoice, GuillotineSplitHeuristic splitMethod);

		/// Computes the ratio of used/total surface area. 0.00 means no space is yet used, 1.00 means the whole bin is used.
		Real Occupancy() const;

		/// Returns the internal list of disjoint rectangles that track the free area of the bin. You may alter this vector
		/// any way desired, as long as the end result still is a list of disjoint rectangles.
		BinPackRectangles &GetFreeRectangles() { return freeRectangles; }

		/// Returns the list of packed rectangles. You may alter this vector at will, for example, you can move a BinPackRectangle from
		/// this list to the Free Rectangles list to free up space on-the-fly, but notice that this causes fragmentation.
		BinPackRectangles &GetUsedRectangles() { return usedRectangles; }

		/// Performs a Rectangle Merge operation. This procedure looks for adjacent free rectangles and merges them if they
		/// can be represented with a single rectangle. Takes up Theta(|freeRectangles|^2) time.
		void MergeFreeList();

	private:
		Int binWidth;
		Int binHeight;

		/// Stores a list of all the rectangles that we have packed so far. This is used only to compute the Occupancy ratio,
		/// so if you want to have the packer consume less memory, this can be removed.
		BinPackRectangles usedRectangles;

		/// Stores a list of rectangles that represents the free area of the bin. This rectangles in this list are disjoint.
		BinPackRectangles freeRectangles;

		/// Goes through the list of free rectangles and finds the best one to place a rectangle of given size into.
		/// Running time is Theta(|freeRectangles|).
		/// @param nodeIndex [out] The index of the free rectangle in the freeRectangles array into which the new
		///		rect was placed.
		/// @return A BinPackRectangle structure that represents the placement of the new rect into the best free rectangle.
		BinPackRectangle FindPositionForNewNode(Int width, Int height, FreeRectChoiceHeuristic rectChoice, Int *nodeIndex);

		static Int ScoreByHeuristic(Int width, Int height, const BinPackRectangle &freeRect, FreeRectChoiceHeuristic rectChoice);
		// The following functions compute (penalty) score values if a rect of the given size was placed into the 
		// given free rectangle. In these score values, smaller is better.

		static Int ScoreBestAreaFit(Int width, Int height, const BinPackRectangle &freeRect);
		static Int ScoreBestShortSideFit(Int width, Int height, const BinPackRectangle &freeRect);
		static Int ScoreBestLongSideFit(Int width, Int height, const BinPackRectangle &freeRect);

		static Int ScoreWorstAreaFit(Int width, Int height, const BinPackRectangle &freeRect);
		static Int ScoreWorstShortSideFit(Int width, Int height, const BinPackRectangle &freeRect);
		static Int ScoreWorstLongSideFit(Int width, Int height, const BinPackRectangle &freeRect);

		/// Splits the given L-shaped free rectangle into two new free rectangles after placedRect has been placed into it.
		/// Determines the split axis by using the given heuristic.
		void SplitFreeRectByHeuristic(const BinPackRectangle &freeRect, const BinPackRectangle &placedRect, GuillotineSplitHeuristic method);

		/// Splits the given L-shaped free rectangle into two new free rectangles along the given fixed split axis.
		void SplitFreeRectAlongAxis(const BinPackRectangle &freeRect, const BinPackRectangle &placedRect, Bool splitHorizontal);
	};

	/** ShelfBinPack implements different bin packing algorithms that use the SHELF data structure. ShelfBinPack
	also uses GuillotineBinPack for the waste map if it is enabled. */
	class ShelfBinPack
	{
	public:
		/// Default ctor initializes a bin of size (0,0). Call Init() to init an instance.
		ShelfBinPack();

		ShelfBinPack(Int width, Int height, Bool useWasteMap);

		/// Clears all previously packed rectangles and starts packing from scratch into a bin of the given size.
		void Init(Int width, Int height, Bool useWasteMap);

		/// Defines different heuristic rules that can be used in the packing process.
		enum ShelfChoiceHeuristic
		{
			ShelfNextFit, ///< -NF: We always put the new rectangle to the last open shelf.
			ShelfFirstFit, ///< -FF: We test each rectangle against each shelf in turn and pack it to the first where it fits.
			ShelfBestAreaFit, ///< -BAF: Choose the shelf with smallest remaining shelf area.
			ShelfWorstAreaFit, ///< -WAF: Choose the shelf with the largest remaining shelf area.
			ShelfBestHeightFit, ///< -BHF: Choose the smallest shelf (height-wise) where the rectangle fits.
			ShelfBestWidthFit, ///< -BWF: Choose the shelf that has the least remaining horizontal shelf space available after packing.
			ShelfWorstWidthFit, ///< -WWF: Choose the shelf that will have most remainining horizontal shelf space available after packing.
		};

		/// Inserts a single rectangle into the bin. The packer might rotate the rectangle, in which case the returned
		/// struct will have the width and height values swapped.
		/// @param method The heuristic rule to use for choosing a shelf if multiple ones are possible.
		BinPackRectangle Insert(Int width, Int height, ShelfChoiceHeuristic method);

		/// Computes the ratio of used surface area to the total bin area.
		Real Occupancy() const;

	private:
		Int binWidth;
		Int binHeight;

		/// Stores the starting y-coordinate of the latest (topmost) shelf.
		Int currentY;

		/// Tracks the total consumed surface area.
		Ulong usedSurfaceArea;

		/// If true, the following GuillotineBinPack structure is used to recover the SHELF data structure from losing space.
		Bool useWasteMap;
		GuillotineBinPack wasteMap;

		/// Describes a horizontal slab of space where rectangles may be placed.
		struct Shelf
		{
			/// The x-coordinate that specifies where the used shelf space ends.
			/// Space between [0, currentX[ has been filled with rectangles, [currentX, binWidth[ is still available for filling.
			Int currentX;

			/// The y-coordinate of where this shelf starts, inclusive.
			Int startY;

			/// Specifices the height of this shelf. The topmost shelf is "open" and its height may grow.
			Int height;

			/// Lists all the rectangles in this shelf.
			BinPackRectangles usedRectangles;
		};

		std::vector<Shelf> shelves;

		/// Parses through all rectangles added to the given shelf and adds the gaps between the rectangle tops and the shelf
		/// ceiling into the waste map. This is called only once when the shelf is being closed and a new one is opened.
		void MoveShelfToWasteMap(Shelf &shelf);

		/// Returns true if the rectangle of size width*height fits on the given shelf, possibly rotated.
		/// @param canResize If true, denotes that the shelf height may be increased to fit the object.
		Bool FitsOnShelf(const Shelf &shelf, Int width, Int height, Bool canResize) const;

		/// Measures and if desirable, flips width and height so that the rectangle fits the given shelf the best.
		/// @param width [in,out] The width of the rectangle.
		/// @param height [in,out] The height of the rectangle.
		void RotateToShelf(const Shelf &shelf, Int &width, Int &height) const;

		/// Adds the rectangle of size width*height into the given shelf, possibly rotated.
		/// @param newNode [out] The added rectangle will be returned here.
		void AddToShelf(Shelf &shelf, Int width, Int height, BinPackRectangle &newNode);

		/// Returns true if there is still room in the bin to start a new shelf of the given height.
		Bool CanStartNewShelf(Int height) const;

		/// Creates a new shelf of the given starting height, which will become the topmost 'open' shelf.
		void StartNewShelf(Int startingHeight);
	};


	/** MaxRectsBinPack implements the MAXRECTS data structure and different bin packing algorithms that 
	use this structure. */
	class MaxRectsBinPack
	{
	public:
		/// Instantiates a bin of size (0,0). Call Init to create a new bin.
		MaxRectsBinPack();

		/// Instantiates a bin of the given size.
		MaxRectsBinPack(Int width, Int height);

		/// (Re)initializes the packer to an empty bin of width x height units. Call whenever
		/// you need to restart with a new bin.
		void Init(Int width, Int height);

		/// Specifies the different heuristic rules that can be used when deciding where to place a new rectangle.
		enum FreeRectChoiceHeuristic
		{
			RectBestShortSideFit, ///< -BSSF: Positions the rectangle against the short side of a free rectangle into which it fits the best.
			RectBestLongSideFit, ///< -BLSF: Positions the rectangle against the long side of a free rectangle into which it fits the best.
			RectBestAreaFit, ///< -BAF: Positions the rectangle into the smallest free rect into which it fits.
			RectBottomLeftRule, ///< -BL: Does the Tetris placement.
			RectContactPointRule ///< -CP: Choosest the placement where the rectangle touches other rects as much as possible.
		};

		/// Inserts the given list of rectangles in an offline/batch mode, possibly rotated.
		/// @param rects The list of rectangles to insert. This vector will be destroyed in the process.
		/// @param dst [out] This list will contain the packed rectangles. The indices will not correspond to that of rects.
		/// @param method The rectangle placement rule to use when packing.
		void Insert(BinPackRectangles &rects, BinPackRectangles &dst, FreeRectChoiceHeuristic method);

		/// Inserts a single rectangle into the bin, possibly rotated.
		BinPackRectangle Insert(Int width, Int height, FreeRectChoiceHeuristic method);

		/// Computes the ratio of used surface area to the total bin area.
		Real Occupancy() const;

	private:
		Int binWidth;
		Int binHeight;

		BinPackRectangles usedRectangles;
		BinPackRectangles freeRectangles;

		/// Computes the placement score for placing the given rectangle with the given method.
		/// @param score1 [out] The primary placement score will be outputted here.
		/// @param score2 [out] The secondary placement score will be outputted here. This isu sed to break ties.
		/// @return This struct identifies where the rectangle would be placed if it were placed.
		BinPackRectangle ScoreRect(Int width, Int height, FreeRectChoiceHeuristic method, Int &score1, Int &score2) const;

		/// Places the given rectangle into the bin.
		void PlaceRect(const BinPackRectangle &node);

		/// Computes the placement score for the -CP variant.
		Int ContactPointScoreNode(Int x, Int y, Int width, Int height) const;

		BinPackRectangle FindPositionForNewNodeBottomLeft(Int width, Int height, Int &bestY, Int &bestX) const;
		BinPackRectangle FindPositionForNewNodeBestShortSideFit(Int width, Int height, Int &bestShortSideFit, Int &bestLongSideFit) const;
		BinPackRectangle FindPositionForNewNodeBestLongSideFit(Int width, Int height, Int &bestShortSideFit, Int &bestLongSideFit) const;
		BinPackRectangle FindPositionForNewNodeBestAreaFit(Int width, Int height, Int &bestAreaFit, Int &bestShortSideFit) const;
		BinPackRectangle FindPositionForNewNodeContactPoint(Int width, Int height, Int &contactScore) const;

		/// @return True if the free node was split.
		Bool SplitFreeNode(BinPackRectangle freeNode, const BinPackRectangle &usedNode);

		/// Goes through the free rectangle list and removes any redundant entries.
		void PruneFreeList();
	};

	/** Implements bin packing algorithms that use the SKYLINE data structure to store the bin contents. Uses
	GuillotineBinPack as the waste map. */
	class SkylineBinPack
	{
	public:
		/// Instantiates a bin of size (0,0). Call Init to create a new bin.
		SkylineBinPack();

		/// Instantiates a bin of the given size.
		SkylineBinPack(Int binWidth, Int binHeight, Bool useWasteMap);

		/// (Re)initializes the packer to an empty bin of width x height units. Call whenever
		/// you need to restart with a new bin.
		void Init(Int binWidth, Int binHeight, Bool useWasteMap);

		/// Defines the different heuristic rules that can be used to decide how to make the rectangle placements.
		enum LevelChoiceHeuristic
		{
			LevelBottomLeft,
			LevelMinWasteFit
		};

		/// Inserts the given list of rectangles in an offline/batch mode, possibly rotated.
		/// @param rects The list of rectangles to insert. This vector will be destroyed in the process.
		/// @param dst [out] This list will contain the packed rectangles. The indices will not correspond to that of rects.
		/// @param method The rectangle placement rule to use when packing.
		void Insert(BinPackRectangles &rects, BinPackRectangles &dst, LevelChoiceHeuristic method);

		/// Inserts a single rectangle into the bin, possibly rotated.
		BinPackRectangle Insert(Int width, Int height, LevelChoiceHeuristic method);

		/// Computes the ratio of used surface area to the total bin area.
		Real Occupancy() const;

	private:
		Int binWidth;
		Int binHeight;

		/// Represents a single level (a horizontal line) of the skyline/horizon/envelope.
		struct SkylineNode
		{
			/// The starting x-coordinate (leftmost).
			Int x;

			/// The y-coordinate of the skyline level line.
			Int y;

			/// The line width. The ending coordinate (inclusive) will be x+width-1.
			Int width;
		};

		std::vector<SkylineNode> skyLine;

		Ulong usedSurfaceArea;

		/// If true, we use the GuillotineBinPack structure to recover wasted areas into a waste map.
		Bool useWasteMap;
		GuillotineBinPack wasteMap;

		BinPackRectangle InsertBottomLeft(Int width, Int height);
		BinPackRectangle InsertMinWaste(Int width, Int height);

		BinPackRectangle FindPositionForNewNodeMinWaste(Int width, Int height, Int &bestHeight, Int &bestWastedArea, Int &bestIndex) const;
		BinPackRectangle FindPositionForNewNodeBottomLeft(Int width, Int height, Int &bestHeight, Int &bestWidth, Int &bestIndex) const;

		Bool RectangleFits(Int skylineNodeIndex, Int width, Int height, Int &y) const;
		Bool RectangleFits(Int skylineNodeIndex, Int width, Int height, Int &y, Int &wastedArea) const;
		Int ComputeWastedArea(Int skylineNodeIndex, Int width, Int height, Int y) const;

		void AddWasteMapArea(Int skylineNodeIndex, Int width, Int height, Int y);

		void AddSkylineLevel(Int skylineNodeIndex, const BinPackRectangle &rect);

		/// Merges all skyline nodes that are at the same level.
		void MergeSkylines();
	};



	class MetaBinPack
	{
	public:
		struct MetaRectangle : public BinPackRectangle 
		{
			MetaRectangle()
			{
				originalWidth = 0;
				originalHeight = 0;
				rotated = 0;
			}

			Int originalWidth;
			Int originalHeight;
			Bool rotated;
		};
		typedef std::vector<MetaRectangle> MetaRectangles;

		MetaBinPack();
		Bool pack(MetaRectangles &rectangles, Int& width, Int& height);

	private:

		ShelfBinPack m_shelf;
		GuillotineBinPack m_guillotine;
		MaxRectsBinPack m_max_rects;
		SkylineBinPack m_skyline;
	
	};


} //namespace rengine

#endif //__RENGINE_RECTANGLE_PACKING_H__
