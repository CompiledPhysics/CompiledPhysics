#pragma once

#include <vector>
#include <cstddef>

#include "Particle.h"
#include "Geometry.h"
#include "Source.h"

namespace mc
{
    /**
     * @brief Abstract base class for particle transport.
     *
     * Uses a `Geometry` and a `Source` to handle the transport (interactions
     * and movement) of particles stored inside an internal list.
     *
     * - `Geometry` and `Source`:
     *   - A `Transporter` uses pointers to an external `Geometry` and `Source`.
     *   - The caller is responsible for creating and destroying these objects.
     *   - This class stores pointers to them, so they must remain valid as long
     *     as the associated Transporter exists.
     *
     * - Particle list:
     *   - The internal particle list stores pointers to Particle objects.
     *   - IMPORTANT: Once a `Particle` pointer has been added to the
     *     `Transporter`'s list, the `Transporter` becomes responsible for
     *      destroying it. It may access or delete these objects at any time.
     *     -> The caller must NOT delete these particle pointers.
     *     -> The `Transporter` destructor deletes all remaining particles.
     */
    class Transporter
    {
    public:
        /// @brief Constructor. Both parameters must be already allocated and point to valid objects.
        /// @param p_source   Pointer to an external particle `Source`.
        /// @param p_geometry Pointer to an external simulation `Geometry`.
        explicit Transporter(Source *p_source, Geometry *p_geometry);

        /// @brief Virtual destructor.
        /// Deletes all remaining particles stored in the internal list.
        /// `Source` and `Geometry` must be deleted by the caller.
        virtual ~Transporter();

        /// @brief Access to the particle list.
        /// The list is sorted by particle energy (from lowest to highest).
        const std::vector<Particle*>& particles() const noexcept;

        /// @brief Returns true if there is at least one particle in the list.
        bool hasParticles() const noexcept;

        /**
         * @brief Initializes the particle list from the `Source`.
         *
         * The `Source` object allocates the requested number of
         * particles and adds them to the `Transporter`'s internal list.
         * The list is then sorted from lowest to highest energy.
         *
         */
        virtual void initializeFromSource(int particleNumber);

        /**
         * @brief Moves the next particle in its current direction.
         *
         * Updates the position of the current particle (the first of the list)
         * depending on the physical properties of the `Geometry` at
         * its current position.
         * The particle may be deleted by the call to `step` (out of boundaries).
         *
         * @return false if there are no more particles to transport
         *         (i.e. the list is empty) and true otherwise.
         */
        virtual bool step() = 0;

        /**
         * @brief Performs an interaction for the current particle.
         *
         * - Uses the `Geometry` to determine the interaction type depending on
         * material properties at the current position.
         * - Computes the deposited energy and record it into the `Geometry`.
         * - Modifies the particle's energy, direction, and possibly its weight.
         * - The particle may be deleted by the call to `interact` (energy too low).
         * - May add any number of new particles to the list.
         */
        virtual void interact() = 0;

        /// @brief Insert a particle inside the internal list.
        ///
        /// The list is sorted from low to high energy.
        /// The insertion is done at the correct index depending on the energy
        /// of the particle.
        ///
        /// @param p_particle A pointer to the `Particle` to be inserted.
        ///
        void insertParticle(Particle* p_particle);

    protected:
        /// @brief Removes the particle at the given index from the list.
        ///
        /// The removed particle is destroyed.
        ///
        /// @param index Index inside the internal particle list.
        ///
        void removeParticle(std::size_t index);

        /// @brief Sorts the particle list by energy.
        ///
        /// Called only by initializeFromSource() for the initial sorting
        /// of particles (lowest to highest energy).
        ///
        void sortParticles();

        /// @brief Sorted list of particle pointers.
        std::vector<Particle*> m_particles;

        /// @brief External simulation geometry.
        Geometry* m_geometry{nullptr};

        /// @brief External particle source.
        Source* m_source{nullptr};
    };

} // namespace mc