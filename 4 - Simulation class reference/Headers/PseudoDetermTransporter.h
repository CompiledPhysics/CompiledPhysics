#pragma once

#include <string>

#include "Transporter.hpp"
#include "CommonTypes.hpp" // 3D vector
#include "ImportanceMap.h"

namespace mc
{
    /**
     * @brief Transporter implementation for a simplified pseudo-deterministic transport algorithm.
     *
     * General idea of the algorithm:
     * - A spherical region of interest (ROI) is defined.
     * - A 3D importance map is used to optimize the simulation efficiency.
     *   - In "generation mode", the transporter builds the
     *     importance map and writes it to a file.
     *     IMPORTANT: this process is slow. Use it once with a short simulation,
     *     then re-use the generated map for a full simulation.
     *   - In "usage mode", the transporter reads an existing importance
     *     map from a file and uses it to optimize the simulation.
     *
     * - Overridden `interact()`:
     *   - If the particle is inside the ROI and has
     *     `isCopied() == true`, it is deleted (for non-biased counting).
     *   - Performs the "base" physical interaction (deposited energy,
     *     direction and energy change, etc.) using the `Geometry` information.
     *   - Uses the importance map at the current position to decide how
     *     many extra copies should be created or if the particle should
     *     undergo a "normal" interaction with no copying instead.
     *   - Creates any number of weighted additional particles depending
     *     on the physical properties of the interaction and the local
     *     importance value. The original particle is flagged with
     *     is isCopied() == true.
     *
     * As with the base `Transporter` class, all particles created
     * (including copies) deleted by the destructor.
     * The importance map is also deleted by the desctuctor.
     */
    class PseudoDetermTransporter : public Transporter
    {
    public:
        /// @brief Operating modes of the pseudo-deterministic transporter.
        enum class Mode
        {
            Generation, ///< Build/update the importance map and write to a file.
            Usage       ///< Read an existing importance map from file and use it.
        };

        /**
         * @brief Constructor.
         *
         * @param source          Pointer to an external particle source.
         * @param geometry        Pointer to an external simulation geometry.
         * @param mode            Operating mode (Generation or Usage).
         * @param roiCenter       Center of the spherical region of interest.
         * @param roiRadius       Radius in cm of the spherical region of interest.
         * @param importanceFile  Path to the importance map file used for
         *                        reading (Usage) or writing (Generation).
         * @throws                std::invalid_argument if source or geometry are 
         *                        nullptr or an inconsistency is detected between
         *                        the region of interest and the (loaded) importance map.
         *
         * Calls setupImportanceMap() to read or create the file to the map.
         */
        PseudoDetermTransporter(Source*            source,
                                Geometry*          geometry,
                                Mode               mode,
                                const Vec3d&       roiCenter,
                                double             roiRadius,
                                const std::string& importanceFile);

        /**
         * @brief Destructor.
         *
         * On top of the base class behaviour, saves the importance map
         * to a file.
         */
        ~PseudoDetermTransporter() override;

        /// @brief Overridden step using pseudo-deterministic transport rules.
        ///
        /// If the particle enters the ROI and has `isCopied == true`, the particle is deleted.
        ///
        bool step() override;

        /// @brief Overridden interact using pseudo-deterministic transport rules.
        ///
        /// - If the particle enters the ROI and has `isCopied == true`, the particle is deleted.
        /// - Performs the base interaction using the Geometry (energy scoring,
        ///   direction and energy updates, etc.).
        /// - Evaluates the local importance from the map.
        /// - Creates additional weighted particle copies depending on the local importance value.
        /// - Adds the isCopied() == true to the original particle.
        /// - Might delete the current particle depending on the local importance.
        void interact() override;

        /// @brief Returns the current operating mode.
        Mode mode() const noexcept { return m_mode; }

        /// @brief Path of the importance map file used in Generation/Usage.
        const std::string& importanceFilePath() const noexcept { return m_importanceFilePath; }

    private:
        /// @brief Checks whether a given position lies inside the ROI.
        bool isInsideRoi(const Vec3d& position) const noexcept;

        /// @brief Accessors for the importance map.
        ImportanceMap* importanceMap() noexcept { return m_importanceMap; }

        /**
         * @brief Spawns copies of the current particle based on the local importance.
         *
         * - Evaluates the importance value at the current particle position
         *   using `importanceMap()->importanceAt(position)`.
         * - Translates this importance value into a number of copies to be created.
         * - Creates the new particles with a proper weighting.
         * - Adds them to the internal particle list.
         *
         * Is used by the overridden `interact()` method.
         */
        void spawnCopiesFromImportance();

        /**
         * @brief Prepares the importance map according to the mode.
         *
         * - In Usage mode:
         *   - Read an existing importance map from `m_importanceFilePath`
         *     into the internal `m_importanceMap`.
         *
         * - In Generation mode:
         *   - Initializes the internal `m_importanceMap` with null values.
         *
         * Called only at object construction.
         */
        void setupImportanceMap();

        Mode m_mode{Mode::Usage};

        /// @brief Center of the spherical region of interest (coordinates in cm).
        Vec3d m_roiCenter{};

        /// @brief Radius of the spherical region of interest (cm).
        double m_roiRadius{0.0};

        /// @brief Non-owning pointer to the importance map.
        ImportanceMap* m_importanceMap{nullptr};

        /// @brief Path to the file used to read/write the importance map.
        std::string m_importanceFilePath;
    };

} // namespace mc