#include "util/mixing_rules/equations_of_state/EquationOfStateMixingRulesManager.hpp"

EquationOfStateMixingRulesManager::EquationOfStateMixingRulesManager(
    const std::string& object_name,
    const tbox::Dimension& dim,
    const int& num_species,
    const MIXING_CLOSURE_MODEL::TYPE& mixing_closure_model,
    const boost::shared_ptr<tbox::Database>& equation_of_state_mixing_rules_db,
    const std::string& equation_of_state_str):
        d_object_name(object_name)
{
    TBOX_ASSERT(!object_name.empty());
    
    if (equation_of_state_str == "IDEAL_GAS")
    {
        d_equation_of_state_type = EQN_STATE::IDEAL_GAS;
        
        d_equation_of_state_mixing_rules.reset(new EquationOfStateMixingRulesIdealGas(
                "d_equation_of_state_mixing_rules",
                dim,
                num_species,
                mixing_closure_model,
                equation_of_state_mixing_rules_db));
    }
    else if (equation_of_state_str == "STIFFENED_GAS")
    {   
        d_equation_of_state_type = EQN_STATE::STIFFENED_GAS;
     
        d_equation_of_state_mixing_rules.reset(new EquationOfStateMixingRulesStiffenedGas(
                "d_equation_of_state_mixing_rules",
                dim,
                num_species,
                mixing_closure_model,
                equation_of_state_mixing_rules_db));
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": "
            << "Unknown equation_of_state/d_equation_of_state string = '"
            << equation_of_state_str
            << "' found in input/restart file."
            << std::endl);        
    }
}


/*
 * Print all characteristics of equation of state manager.
 */
void
EquationOfStateMixingRulesManager::printClassData(std::ostream& os) const
{
    os << "\nPrint EquationOfStateMixingRulesManager object..."
       << std::endl;
    
    os << std::endl;
    
    os << "EquationOfStateMixingRulesManager: this = "
       << (EquationOfStateMixingRulesManager *)this
       << std::endl;
    
    os << "d_object_name = "
       << d_object_name
       << std::endl;
    
    os << "d_equation_of_state_type = "
       << d_equation_of_state_type
       << std::endl;
    
    os << "................................................................................";
    d_equation_of_state_mixing_rules->printClassData(os);
}
