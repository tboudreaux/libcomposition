/* ***********************************************************************
//
//   Copyright (C) 2025 -- The 4D-STAR Collaboration
//   File Author: Emily Boudreaux
//   Last Modified: March 26, 2025
//
//   4DSSE is free software; you can use it and/or modify
//   it under the terms and restrictions the GNU General Library Public
//   License version 3 (GPLv3) as published by the Free Software Foundation.
//
//   4DSSE is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU Library General Public License for more details.
//
//   You should have received a copy of the GNU Library General Public License
//   along with this software; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// *********************************************************************** */
#pragma once

#include <string>
#include <unordered_map>
#include <set>

#include <utility>

#include "fourdst/config/config.h"
#include "fourdst/logging/logging.h"
#include "fourdst/composition/atomicSpecies.h"

namespace fourdst::composition {
    struct CanonicalComposition {
        double X = 0.0; ///< Mass fraction of Hydrogen.
        double Y = 0.0; ///< Mass fraction of Helium.
        double Z = 0.0; ///< Mass fraction of Metals.

        friend std::ostream& operator<<(std::ostream& os, const CanonicalComposition& composition) {
            os << "<CanonicalComposition: "
               << "X = " << composition.X << ", "
               << "Y = " << composition.Y << ", "
               << "Z = " << composition.Z << ">";
            return os;
        }
    };

    /**
     * @brief Represents the global composition of a system. This tends to be used after finalize and is primarily for internal use.
     */
    struct GlobalComposition {
        double specificNumberDensity; ///< The specific number density of the composition (\sum_{i} X_i m_i. Where X_i is the number fraction of the ith species and m_i is the mass of the ith species).
        double meanParticleMass; ///< The mean particle mass of the composition (\sum_{i} \frac{n_i}{m_i}. where n_i is the number fraction of the ith species and m_i is the mass of the ith species).

        // Overload the output stream operator for GlobalComposition
        friend std::ostream& operator<<(std::ostream& os, const GlobalComposition& comp);
    };

    /**
     * @brief Represents an entry in the composition with a symbol and mass fraction.
     */
    struct CompositionEntry {
        std::string m_symbol; ///< The chemical symbol of the species.
        atomic::Species m_isotope; ///< The isotope of the species.
        bool m_massFracMode = true; ///< The mode of the composition entry. True if mass fraction, false if number fraction.   

        double m_massFraction = 0.0; ///< The mass fraction of the species.
        double m_numberFraction = 0.0; ///< The number fraction of the species.
        double m_relAbundance = 0.0; ///< The relative abundance of the species for converting between mass and number fractions.

        bool m_initialized = false; ///< True if the composition entry has been initialized.

        /**
         * @brief Default constructor.
         */
        CompositionEntry();

        /**
         * @brief Constructs a CompositionEntry with the given symbol and mode.
         * @param symbol The chemical symbol of the species.
         * @param massFracMode True if mass fraction mode, false if number fraction mode.
         * *Example Usage:*
         * @code
         * CompositionEntry entry("H", true);
         * @endcode
         */
        explicit CompositionEntry(const std::string& symbol, bool massFracMode=true);

        /**
         * @brief Copy constructor.
         * @param entry The CompositionEntry to copy.
         */
        CompositionEntry(const CompositionEntry& entry);

        /**
         * @brief Sets the species for the composition entry.
         * @param symbol The chemical symbol of the species.
         */
        void setSpecies(const std::string& symbol);

        /**
         * @brief Gets the chemical symbol of the species.
         * @return The chemical symbol of the species.
         */
        [[nodiscard]] std::string symbol() const;

        /**
         * @brief Gets the mass fraction of the species.
         * @return The mass fraction of the species.
         */
        [[nodiscard]] double mass_fraction() const;

        /**
         * @brief Gets the mass fraction of the species given the mean molar mass.
         * @param meanMolarMass The mean molar mass.
         * @return The mass fraction of the species.
         */
        [[nodiscard]] double mass_fraction(double meanMolarMass) const;

        /**
         * @brief Gets the number fraction of the species.
         * @return The number fraction of the species.
         */
        [[nodiscard]] double number_fraction() const;

        /**
         * @brief Gets the number fraction of the species given the total moles.
         * @param totalMoles The total moles.
         * @return The number fraction of the species.
         */
        [[nodiscard]] double number_fraction(double totalMoles) const;

        /**
         * @brief Gets the relative abundance of the species.
         * @return The relative abundance of the species.
         */
        [[nodiscard]] double rel_abundance() const;

        /**
         * @brief Gets the isotope of the species.
         * @return The isotope of the species.
         */
        [[nodiscard]] atomic::Species isotope() const;

        /**
         * @brief Gets the mode of the composition entry.
         * @return True if mass fraction mode, false if number fraction mode.
         */
        [[nodiscard]] bool getMassFracMode() const;

        /**
         * @brief Sets the mass fraction of the species.
         * @param mass_fraction The mass fraction to set.
         */
        void setMassFraction(double mass_fraction);

        /**
         * @brief Sets the number fraction of the species.
         * @param number_fraction The number fraction to set.
         */
        void setNumberFraction(double number_fraction);

        /**
         * @brief Sets the mode to mass fraction mode.
         * @param meanMolarMass The mean molar mass.
         * @return True if the mode was successfully set, false otherwise.
         */
        bool setMassFracMode(double meanMolarMass);

        /**
         * @brief Sets the mode to number fraction mode.
         * @param totalMoles The total moles.
         * @return True if the mode was successfully set, false otherwise.
         */
        bool setNumberFracMode(double totalMoles);

        /**
         * @brief Overloaded output stream operator for CompositionEntry.
         * @param os The output stream.
         * @param entry The CompositionEntry to output.
         * @return The output stream.
         */
        friend std::ostream& operator<<(std::ostream& os, const CompositionEntry& entry);
    };

    /**
     * @brief Manages the composition of elements.
     * @details The composition is a collection of elements with their respective mass fractions.
     * The general purpose of this class is to provide a standardized interface for managing the composition of
     * any part of 4DSSE. There are a few rules when using this class.
     * - Only species in the atomicSpecies.h database can be used. There are 1000s (All species from AME2020) in there so it should not be a problem.
     * - Before a mass fraction can be set with a particular instance of Composition, the symbol must be registered. (i.e. register He-3 before setting its mass fraction)
     * - Before any composition information can be retrieved (e.g. getComposition), the composition must be finalized (call to .finalize()). This checks if the total mass fraction sums to approximately 1 (within 1 part in 10^8)
     * - Any changes made to the composition after finalization will "un-finalize" the composition. This means that the composition must be finalized again before any information can be retrieved.
     * - The mass fraction of any individual species must be no more than 1 and no less than 0.
     * - The only exception to the finalize rule is if the composition was constructed with symbols and mass fractions at instantiation time. In this case, the composition is automatically finalized.
     * however, this means that the composition passed to the constructor must be valid.
     * 
     * *Example Usage:* Constructing a finalized composition with symbols and mass fractions:
     * @code
     * std::vector<std::string> symbols = {"H", "He"};
     * std::vector<double> mass_fractions = {0.7, 0.3};
     * Composition comp(symbols, mass_fractions);
     * @endcode
     * *Example Usage:* Constructing a composition with symbols and finalizing it later:
     * @code
     * std::vector<std::string> symbols = {"H", "He"};
     * Composition comp(symbols);
     * comp.setComposition("H", 0.7);
     * comp.setComposition("He", 0.3);
     * comp.finalize();
     * @endcode
     */
    class Composition {
    private:
        fourdst::config::Config& m_config = fourdst::config::Config::getInstance();
        fourdst::logging::LogManager& m_logManager = fourdst::logging::LogManager::getInstance();
        quill::Logger* m_logger = m_logManager.getLogger("log");

        bool m_finalized = false; ///< True if the composition is finalized.
        double m_specificNumberDensity = 0.0; ///< The specific number density of the composition (\sum_{i} X_i m_i. Where X_i is the number fraction of the ith species and m_i is the mass of the ith species).
        double m_meanParticleMass = 0.0; ///< The mean particle mass of the composition (\sum_{i} \frac{n_i}{m_i}. where n_i is the number fraction of the ith species and m_i is the mass of the ith species).
        bool m_massFracMode = true; ///< True if mass fraction mode, false if number fraction mode.

        std::set<std::string> m_registeredSymbols; ///< The registered symbols.
        std::unordered_map<std::string, CompositionEntry> m_compositions; ///< The compositions.

        /**
         * @brief Checks if the given symbol is valid. 
         * @details A symbol is valid if it is in the atomic species database (species in atomicSpecies.h). These include all the isotopes from AME2020.
         * @param symbol The symbol to check.
         * @return True if the symbol is valid, false otherwise.
         */
        static bool isValidSymbol(const std::string& symbol);

        /**
         * @brief Checks if the given mass fractions are valid.
         * @param fractions The mass fractions to check.
         * @return True if the mass fractions are valid, false otherwise.
         */
        [[nodiscard]] bool isValidComposition(const std::vector<double>& fractions) const;

        /**
         * @brief Validates the given mass fractions.
         * @param fractions The mass fractions to validate.
         * @throws std::invalid_argument if the mass fractions are invalid.
         */
        void validateComposition(const std::vector<double>& fractions) const;

        /**
         * @brief Finalizes the composition in mass fraction mode.
         * @param norm If true, the composition will be normalized to sum to 1.
         * @return True if the composition is successfully finalized, false otherwise.
         */
        bool finalizeMassFracMode(bool norm);

        /**
         * @brief Finalizes the composition in number fraction mode.
         * @param norm If true, the composition will be normalized to sum to 1.
         * @return True if the composition is successfully finalized, false otherwise.
         */
        bool finalizeNumberFracMode(bool norm);

    public:
        /**
         * @brief Default constructor.
         */
        Composition() = default;

        /**
         * @brief Default destructor.
         */
        ~Composition() = default;

        /**
         * @brief Finalizes the composition.
         * @param norm If true, the composition will be normalized to sum to 1 [Default False]
         * @return True if the composition is successfully finalized, false otherwise.
         */
        bool finalize(bool norm=false);

        /**
         * @brief Constructs a Composition with the given symbols.
         * @param symbols The symbols to initialize the composition with.
         * *Example Usage:*
         * @code
         * std::vector<std::string> symbols = {"H", "O"};
         * Composition comp(symbols);
         * @endcode
         */
        explicit Composition(const std::vector<std::string>& symbols);

        /**
         * @brief Constructs a Composition with the given symbols as a set.
         * @param symbols The symbols to initialize the composition with.
         * *Example Usage:*
         * @code
         * std::set<std::string> symbols = {"H", "O"};
         * Composition comp(symbols);
         * @endcode
         */
        explicit Composition(const std::set<std::string>& symbols);

        /**
         * @brief Constructs a Composition with the given symbols and mass fractions.
         * @param symbols The symbols to initialize the composition with.
         * @param mass_fractions The mass fractions corresponding to the symbols.
         * @param massFracMode True if mass fraction mode, false if number fraction mode.
         * *Example Usage:*
         * @code
         * std::vector<std::string> symbols = {"H", "O"};
         * std::vector<double> mass_fractions = {0.1, 0.9};
         * Composition comp(symbols, mass_fractions);
         * @endcode
         */
        Composition(const std::vector<std::string>& symbols, const std::vector<double>& mass_fractions, bool massFracMode=true);

        /**
         * @brief Constructs a Composition from another Composition.
         * @param composition The Composition to copy.
         */
        Composition(const Composition& composition);

        Composition& operator=(Composition const& other);

        /**
         * @brief Registers a new symbol.
         * @param symbol The symbol to register.
         * @param massFracMode True if mass fraction mode, false if number fraction mode.
         * *Example Usage:*
         * @code
         * Composition comp;
         * comp.registerSymbol("H");
         * @endcode
         */
        void registerSymbol(const std::string& symbol, bool massFracMode=true);

        /**
         * @brief Registers multiple new symbols.
         * @param symbols The symbols to register.
         * @param massFracMode True if mass fraction mode, false if number fraction mode.
         * *Example Usage:*
         * @code
         * std::vector<std::string> symbols = {"H", "O"};
         * Composition comp;
         * comp.registerSymbol(symbols);
         * @endcode
         */
        void registerSymbol(const std::vector<std::string>& symbols, bool massFracMode=true);

        /**
         * @brief Register a new species (will extract the symbol from the species).
         * @param species The species to register.
         * @param massFracMode True if mass fraction mode, false if number fraction mode.
         * *Example Usage:*
         * @code
         * #include "fourdst/atomic/species.h"
         *
         * Composition comp;
         * comp.registerSpecies(H_1);
         *
         * #@endcode
         */
        void registerSpecies(const fourdst::atomic::Species& species, bool massFracMode=true);


        /**
         * @brief Register a vector of new species (will extract the symbol from the species).
         * @param species The vector of species to register.
         * @param massFracMode True if mass fraction mode, false if number fraction mode.
         * *Example Usage:*
         * @code
         * #include "fourdst/atomic/species.h"
         *
         * Composition comp;
         * comp.registerSpecies({H_1, He_4, O_16}, false); // Will now be in number fraction mode.
         *
         * #@endcode
         */
        void registerSpecies(const std::vector<fourdst::atomic::Species>& species, bool massFracMode=true);


        /**
         * @brief Gets the registered symbols.
         * @return A set of registered symbols.
         */
        [[nodiscard]] std::set<std::string> getRegisteredSymbols() const;

        /**
         * @brief Get a set of all species that are registered in the composition.
         * @return species that are registered in the composition.
         */
        [[nodiscard]] std::set<fourdst::atomic::Species> getRegisteredSpecies() const;

        /**
         * @brief Sets the mass fraction for a given symbol.
         * @param symbol The symbol to set the mass fraction for.
         * @param mass_fraction The mass fraction to set.
         * @return The mass fraction that was set.
         * *Example Usage:*
         * @code
         * Composition comp;
         * comp.setMassFraction("H", 0.1);
         * @endcode
         */
        double setMassFraction(const std::string& symbol, const double& mass_fraction);

        /**
         * @brief Sets the mass fraction for multiple symbols.
         * @param symbols The symbols to set the mass fraction for.
         * @param mass_fractions The mass fractions corresponding to the symbols.
         * @return A vector of mass fractions that were set.
         * *Example Usage:*
         * @code
         * std::vector<std::string> symbols = {"H", "O"};
         * std::vector<double> mass_fractions = {0.1, 0.9};
         * Composition comp;
         * comp.setMassFraction(symbols, mass_fractions);
         * @endcode
         */
        std::vector<double> setMassFraction(const std::vector<std::string>& symbols, const std::vector<double>& mass_fractions);

        /**
         * @brief Sets the mass fraction for a given species.
         * @param species Species to set the mass fraction for.
         * @param mass_fraction Mass fraction to set for the species.
         * @return The previous mass fraction that was set for the species.
         *
         * @throws std::runtime_error if the species is not registered or if the composition is in number fraction mode.
         * @throws std::logic_error if the mass fraction is not between 0 and 1.
         */
        double setMassFraction(const fourdst::atomic::Species& species, const double& mass_fraction);

        /**
         * @brief Sets the mass fraction for multiple species.
         * @param species The vector of species to set the mass fractions for.
         * @param mass_fractions The vector of mass fractions corresponding to the species.
         * @return A vector of mass fractions that were set.
         *
         * @throws std::runtime_error if the species are not registered or if the composition is in number fraction mode.
         * @throws std::logic_error if any mass fraction is not between 0 and 1.
         */
        std::vector<double> setMassFraction(const std::vector<fourdst::atomic::Species>& species, const std::vector<double>& mass_fractions);

        /**
         * @brief Sets the number fraction for a given symbol.
         * @param symbol The symbol to set the number fraction for.
         * @param number_fraction The number fraction to set.
         * @return The number fraction that was set.
         */
        double setNumberFraction(const std::string& symbol, const double& number_fraction);

        /**
         * @brief Sets the number fraction for multiple symbols.
         * @param symbols The symbols to set the number fraction for.
         * @param number_fractions The number fractions corresponding to the symbols.
         * @return A vector of number fractions that were set.
         */
        std::vector<double> setNumberFraction(const std::vector<std::string>& symbols, const std::vector<double>& number_fractions);

        /**
         * @brief Sets the number fraction for a given species.
         * @param species The species to set the number fraction for.
         * @param number_fraction The number fraction to set for the species.
         * @return The previous number fraction that was set for the species.
         *
         * @throws std::runtime_error if the species is not registered or if the composition is in mass fraction mode.
         * @throws std::logic_error if the number fraction is not between 0 and 1.
         */
        double setNumberFraction(const fourdst::atomic::Species& species, const double& number_fraction);

        /**
         * @brief Sets the number fraction for multiple species.
         * @param species The vector of species to set the number fractions for.
         * @param number_fractions The vector of number fractions corresponding to the species.
         * @return The vector of number fractions that were set.
         *
         * @throws std::runtime_error if the species are not registered or if the composition is in mass fraction mode.
         * @throws std::logic_error if any number fraction is not between 0 and 1.
         */
        std::vector<double> setNumberFraction(const std::vector<fourdst::atomic::Species>& species, const std::vector<double>& number_fractions);

        /** 
        * @brief Mix two compositions together with a given fraction.
        * @param other The other composition to mix with.
        * @param fraction The fraction of the other composition to mix with. This is the fraction of the other composition wrt. to the current. i.e. fraction=1 would mean that 50% of the new composition is from the other and 50% from the current.
        */
        [[nodiscard]] Composition mix(const Composition& other, double fraction) const;

        /**
         * @brief Gets the mass fractions of all compositions.
         * @return An unordered map of compositions with their mass fractions.
         */
        [[nodiscard]] std::unordered_map<std::string, double> getMassFraction() const;

        /**
         * @brief Gets the mass fraction for a given symbol.
         * @param symbol The symbol to get the mass fraction for.
         * @return The mass fraction for the given symbol.
         */
        [[nodiscard]] double getMassFraction(const std::string& symbol) const;

        /**
         * @brief Gets the mass fraction for a given isotope.
         * @param species The isotope to get the mass fraction for.
         * @return The mass fraction for the given isotope.
         *
         * @throws std::runtime_error if the isotope is not registered in the composition.
         */
        [[nodiscard]] double getMassFraction(const fourdst::atomic::Species& species) const;

        /**
         * @brief Gets the number fraction for a given symbol.
         * @param symbol The symbol to get the number fraction for.
         * @return The number fraction for the given symbol.
         */
        [[nodiscard]] double getNumberFraction(const std::string& symbol) const;

        /**
         * @brief Gets the number fraction for a given isotope.
         * @param species The isotope to get the number fraction for.
         * @return The number fraction for the given isotope.
         *
         * @throws std::runtime_error if the isotope is not registered in the composition.
         */
        [[nodiscard]] double getNumberFraction(const fourdst::atomic::Species& species) const;

        /**
         * @brief Gets the number fractions of all compositions.
         * @return An unordered map of compositions with their number fractions.
         */
        [[nodiscard]] std::unordered_map<std::string, double> getNumberFraction() const;

        /**
        * @brief Gets the molar abundance for a given symbol.
        * @param symbol The symbol to get the molar abundance for.
        * @return The molar abundance for the given symbol.
        */
        [[nodiscard]] double getMolarAbundance(const std::string& symbol) const;

        /**
         * @brief Gets the molar abundance for a given isotope.
         * @param species The isotope to get the molar abundance for.
         * @return The molar abundance for the given isotope.
         *
         * @throws std::runtime_error if the isotope is not registered in the composition.
         */
        [[nodiscard]] double getMolarAbundance(const fourdst::atomic::Species& species) const;

        /**
         * @brief Gets the composition entry and global composition for a given symbol.
         * @param symbol The symbol to get the composition for.
         * @return A pair containing the CompositionEntry and GlobalComposition for the given symbol.
         */
        [[nodiscard]] std::pair<CompositionEntry, GlobalComposition> getComposition(const std::string& symbol) const;

        [[nodiscard]] std::pair<CompositionEntry, GlobalComposition> getComposition(const fourdst::atomic::Species& species) const;

        /**
         * @brief Gets all composition entries and the global composition.
         * @return A pair containing an unordered map of CompositionEntries and the GlobalComposition.
         */
        [[nodiscard]] std::pair<std::unordered_map<std::string, CompositionEntry>, GlobalComposition> getComposition() const;

        /**
         * @brief Compute the mean particle mass of the composition.
         * @return Mean particle mass in g.
         */
        [[nodiscard]] double getMeanParticleMass() const;

        /**
         * @brief Compute the mean atomic mass number of the composition.
         * @return Mean atomic mass number.
         */
        [[nodiscard]] double getMeanAtomicNumber() const;

        /**
         * @brief Gets a subset of the composition.
         * @param symbols The symbols to include in the subset.
         * @param method The method to use for the subset (default is "norm").
         * @return A Composition object containing the subset.
         */
        [[nodiscard]] Composition subset(const std::vector<std::string>& symbols, const std::string& method="norm") const;

        /** 
        * @brief Check if a symbol is registered.
        * @param symbol The symbol to check.
        * @return True if the symbol is registered, false otherwise.
        */
        [[nodiscard]] bool hasSymbol(const std::string& symbol) const;

        [[nodiscard]] bool contains(const fourdst::atomic::Species& isotope) const;

        /**
        * @brief Sets the composition mode.
        * @param massFracMode True if mass fraction mode, false if number fraction mode.
        */
        void setCompositionMode(bool massFracMode);

        /**
         * @brief Gets the current canonical composition (X, Y, Z).
         * @param harsh If true, this will throw an error if X-Y != Z where Z is computed as the sum of all other elements.
         * @return True if mass fraction mode, false if number fraction mode.
         *
         * @throws std::runtime_error if the composition is not finalized or if the canonical composition cannot be computed.
         * @throws std::runtime_error if harsh is true and the canonical composition is not valid.
         */
        [[nodiscard]] CanonicalComposition getCanonicalComposition(bool harsh=false) const;

        /**
         * @brief Overloaded output stream operator for Composition.
         * @param os The output stream.
         * @param composition The Composition to output.
         * @return The output stream.
         */
        friend std::ostream& operator<<(std::ostream& os, const Composition& composition);

        // Overload the + operator to call mix with a fraction of 0.5
        /**
        * @brief Overloads the + operator to mix two compositions together with a fraction of 0.5.
        * @param other The other composition to mix with.
        * @return The mixed composition.
        */
        Composition operator+(const Composition& other) const;

        auto begin() {
            return m_compositions.begin();
        }

        auto begin() const {
            return m_compositions.cbegin();
        }

        auto end() {
            return m_compositions.end();
        }

        auto end() const {
            return m_compositions.cend();
        }

    };
}; // namespace fourdst::composition
