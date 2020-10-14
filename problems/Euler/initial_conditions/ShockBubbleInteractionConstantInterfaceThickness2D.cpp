#include "apps/Euler/EulerInitialConditions.hpp"

/*
 * Set the data on the patch interior to some initial values.
 */
void
EulerInitialConditions::initializeDataOnPatch(
    hier::Patch& patch,
    const std::vector<HAMERS_SHARED_PTR<pdat::CellData<double> > >& conservative_variables,
    const double data_time,
    const bool initial_time)
{
    NULL_USE(data_time);
    
    if (d_project_name != "2D shock-bubble interaction with constant interface thickness")
    {
        TBOX_ERROR(d_object_name
            << ": "
            << "Can only initialize data for 'project_name' = "
            << "'2D shock-bubble interaction with constant interface thickness'!\n"
            << "'project_name' = '"
            << d_project_name
            << "' is given."
            << std::endl);
    }
    
    if (d_dim != tbox::Dimension(2))
    {
        TBOX_ERROR(d_object_name
            << ": "
            << "Dimension of problem should be 2!"
            << std::endl);
    }
    
    if (d_flow_model_type != FLOW_MODEL::FIVE_EQN_ALLAIRE)
    {
        TBOX_ERROR(d_object_name
            << ": "
            << "Flow model should be five-equation model by Allaire!"
            << std::endl);
    }
    
    if (d_flow_model->getNumberOfSpecies() != 2)
    {
        TBOX_ERROR(d_object_name
            << ": "
            << "Number of species should be 2!"
            << std::endl);
    }
    
    if (initial_time)
    {
        const HAMERS_SHARED_PTR<geom::CartesianPatchGeometry> patch_geom(
            HAMERS_SHARED_PTR_CAST<geom::CartesianPatchGeometry, hier::PatchGeometry>(
                patch.getPatchGeometry()));
        
#ifdef HAMERS_DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(patch_geom);
#endif
        
        const double* const dx = patch_geom->getDx();
        const double* const patch_xlo = patch_geom->getXLower();
        
        // Get the dimensions of box that covers the interior of Patch.
        hier::Box patch_box = patch.getBox();
        const hier::IntVector patch_dims = patch_box.numberCells();
        
        /*
         * Initialize data for a 2D shock-bubble interaction with constant interface problem.
         */
        
        HAMERS_SHARED_PTR<pdat::CellData<double> > partial_density = conservative_variables[0];
        HAMERS_SHARED_PTR<pdat::CellData<double> > momentum        = conservative_variables[1];
        HAMERS_SHARED_PTR<pdat::CellData<double> > total_energy    = conservative_variables[2];
        HAMERS_SHARED_PTR<pdat::CellData<double> > volume_fraction = conservative_variables[3];
        
        double* Z_rho_1   = partial_density->getPointer(0);
        double* Z_rho_2   = partial_density->getPointer(1);
        double* rho_u     = momentum->getPointer(0);
        double* rho_v     = momentum->getPointer(1);
        double* E         = total_energy->getPointer(0);
        double* Z_1       = volume_fraction->getPointer(0);
        double* Z_2       = volume_fraction->getPointer(1);
        
        // Define of the characteristic lengths of the problem.
        const double D = double(1);
        const double epsilon_i = double(3)/double(400);
        
        // species 0: He
        // species 1: air
        const double gamma_0 = double(1.648);
        const double gamma_1 = double(1.4);
        
        // He, pre-shock condition.
        const double rho_He = double(0.1819);
        const double u_He   = double(0);
        const double v_He   = double(0);
        const double p_He   = double(1)/double(1.4);
        const double Z_He   = double(1);
        
        // air, pre-shock condition.
        const double rho_pre = double(1);
        const double u_pre   = double(0);
        const double v_pre   = double(0);
        const double p_pre   = double(1)/double(1.4);
        const double Z_pre   = double(0);
        
        // air, post-shock condition.
        const double rho_post = double(1.3764);
        const double u_post   = double(-0.3336);
        const double v_post   = double(0);
        const double p_post   = double(1.5698)/double(1.4);
        const double Z_post   = double(0);
        
        for (int j = 0; j < patch_dims[1]; j++)
        {
            for (int i = 0; i < patch_dims[0]; i++)
            {
                // Compute index into linear data array.
                int idx_cell = i + j*patch_dims[0];
                
                // Compute the coordinates.
                double x[2];
                x[0] = patch_xlo[0] + (double(i) + double(1)/double(2))*dx[0];
                x[1] = patch_xlo[1] + (double(j) + double(1)/double(2))*dx[1];
                
                if (x[0] > double(9)/double(2)*D)
                {
                    Z_rho_1[idx_cell] = double(0);
                    Z_rho_2[idx_cell] = rho_post;
                    rho_u[idx_cell]   = rho_post*u_post;
                    rho_v[idx_cell]   = rho_post*v_post;
                    E[idx_cell]       = p_post/(gamma_1 - double(1)) + double(1)/double(2)*rho_post*
                        (u_post*u_post + v_post*v_post);
                    Z_1[idx_cell]     = Z_post;
                    Z_2[idx_cell]     = double(1) - Z_post;
                }
                else
                {
                    // Compute the distance from the initial material interface.
                    const double dR = sqrt(pow(x[0] - double(7)/double(2), 2) + x[1]*x[1]) -
                        double(1)/double(2)*D;
                    
                    const double f_sm = double(1)/double(2)*(double(1) + erf(dR/epsilon_i));
                    
                    // Smooth the primitive quantity.
                    const double Z_rho_1_i = rho_He*(double(1) - f_sm);
                    const double Z_rho_2_i = rho_pre*f_sm;
                    const double u_i       = u_He*(double(1) - f_sm) + u_pre*f_sm;
                    const double v_i       = v_He*(double(1) - f_sm) + v_pre*f_sm;
                    const double p_i       = p_He*(double(1) - f_sm) + p_pre*f_sm;
                    const double Z_1_i     = Z_He*(double(1) - f_sm) + Z_pre*f_sm;
                    
                    const double rho_i = Z_rho_1_i + Z_rho_2_i;
                    const double Z_2_i = double(1) - Z_1_i;
                    
                    const double gamma = double(1)/(Z_1_i/(gamma_0 - double(1)) + Z_2_i/(gamma_1 - double(1))) +
                        double(1);
                    
                    Z_rho_1[idx_cell] = Z_rho_1_i;
                    Z_rho_2[idx_cell] = Z_rho_2_i;
                    rho_u[idx_cell]   = rho_i*u_i;
                    rho_v[idx_cell]   = rho_i*v_i;
                    E[idx_cell]       = p_i/(gamma - double(1)) + double(1)/double(2)*rho_i*
                        (u_i*u_i + v_i*v_i);
                    Z_1[idx_cell]     = Z_1_i;
                    Z_2[idx_cell]     = Z_2_i;
                }
            }
        }
    }
}
