#ifndef EQUATION_OF_THERMAL_CONDUCTIVITY_MIXING_RULES_PRANDTL_HPP
#define EQUATION_OF_THERMAL_CONDUCTIVITY_MIXING_RULES_PRANDTL_HPP

#include "util/mixing_rules/equations_of_thermal_conductivity/EquationOfThermalConductivityMixingRules.hpp"

#include "util/mixing_rules/equations_of_thermal_conductivity/Prandtl/EquationOfThermalConductivityPrandtl.hpp"
#include "util/mixing_rules/equations_of_shear_viscosity/EquationOfShearViscosityMixingRulesManager.hpp"

class EquationOfThermalConductivityMixingRulesPrandtl: public EquationOfThermalConductivityMixingRules
{
    public:
        EquationOfThermalConductivityMixingRulesPrandtl(
            const std::string& object_name,
            const tbox::Dimension& dim,
            const int& num_species,
            const MIXING_CLOSURE_MODEL::TYPE& mixing_closure_model,
            const boost::shared_ptr<tbox::Database>& equation_of_thermal_conductivity_mixing_rules_db);
        
        ~EquationOfThermalConductivityMixingRulesPrandtl() {}
        
        /*
         * Return the boost::shared_ptr to the equation of thermal conductivity.
         */
        const boost::shared_ptr<EquationOfThermalConductivity>&
        getEquationOfThermalConductivity(const int species_index = 0) const
        {
            NULL_USE(species_index);
            return d_equation_of_thermal_conductivity;
        }
        
        /*
         * Print all characteristics of the equation of thermal conductivity mixing rules class.
         */
        void
        printClassData(std::ostream& os) const;
        
        /*
         * Put the characteristics of the equation of thermal conductivity mixing rules class into the restart
         * database.
         */
        void
        putToRestart(
            const boost::shared_ptr<tbox::Database>& restart_db) const;
        
        /*
         * Compute the thermal conductivity of the mixture with isothermal and isobaric equilibrium assumptions.
         */
        double
        getThermalConductivity(
            const double* const pressure,
            const double* const temperature,
            const std::vector<const double*>& mass_fractions) const;
        
        /*
         * Compute the thermal conductivity of the mixture with isothermal and isobaric equilibrium assumptions.
         */
        void
        computeThermalConductivity(
            boost::shared_ptr<pdat::CellData<double> >& data_thermal_conductivity,
            const boost::shared_ptr<pdat::CellData<double> >& data_pressure,
            const boost::shared_ptr<pdat::CellData<double> >& data_temperature,
            const boost::shared_ptr<pdat::CellData<double> >& data_mass_fractions,
            const hier::Box& domain) const;
        
        /*
         * Get the number of molecular properties of a species.
         */
        int
        getNumberOfSpeciesMolecularProperties(const int species_index = 0) const
        {
            NULL_USE(species_index);
            return (3 +
                d_equation_of_shear_viscosity_mixing_rules->
                    getNumberOfSpeciesMolecularProperties());
        }
        
        /*
         * Get the molecular properties of a species.
         */
        void
        getSpeciesMolecularProperties(
            std::vector<double*>& species_molecular_properties,
            const int species_index = 0) const;
        
    private:
        /*
         * Specific heats at constant pressure of different species.
         */
        std::vector<double> d_species_c_p;
        
        /*
         * Prandtl number of different species.
         */
        std::vector<double> d_species_Pr;
        
        /*
         * Molecular weight of different species.
         */
        std::vector<double> d_species_M;
        
        /*
         * A string variable to describe the equation of shear viscosity used.
         */
        std::string d_equation_of_shear_viscosity_str;
        
        /*
         * boost::shared_ptr to EquationOfShearViscosityMixingRules.
         */
        boost::shared_ptr<EquationOfShearViscosityMixingRules> d_equation_of_shear_viscosity_mixing_rules;
        
        /*
         * boost::shared_ptr to EquationOfShearViscosityMixingRulesManager.
         */
        boost::shared_ptr<EquationOfShearViscosityMixingRulesManager> d_equation_of_shear_viscosity_mixing_rules_manager;
        
        /*
         * boost::shared_ptr to EquationOfThermalConductivity.
         */
        boost::shared_ptr<EquationOfThermalConductivity> d_equation_of_thermal_conductivity;
        
};
    

#endif /* EQUATION_OF_THERMAL_CONDUCTIVITY_MIXING_RULES_PRANDTL_HPP */
