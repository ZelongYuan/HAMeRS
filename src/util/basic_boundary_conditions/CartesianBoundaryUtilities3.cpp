/*************************************************************************
 *
 * This file is modified from CartesianBoundaryUtilities3.c of the SAMRAI
 * distribution. For full copyright information, see COPYRIGHT and
 * COPYING.LESSER of SAMRAI distribution.
 *
 * Copyright:     (c) 1997-2014 Lawrence Livermore National Security, LLC
 * Description:   Utility routines for manipulating 3D Cartesian boundary data
 *
 ************************************************************************/

#include "util/basic_boundary_conditions/CartesianBoundaryUtilities3.hpp"

#include "SAMRAI/geom/CartesianPatchGeometry.h"
#include "SAMRAI/hier/BoundaryBox.h"
#include "SAMRAI/pdat/CellIndex.h"
#include "SAMRAI/tbox/Utilities.h"
#include "SAMRAI/tbox/MathUtilities.h"

#include "util/basic_boundary_conditions/CartesianBoundaryDefines.hpp"

/*
 * This function reads 3D boundary data from given input database.
 * The integer boundary condition types are placed in the integer
 * arrays supplied by the caller (typically, the concrete BoundaryStrategy
 * provided).  When DIRICHLET or NEUMANN conditions are specified, control
 * is passed to the BoundaryStrategy to read the boundary state data
 * specific to the problem.
 *
 * Errors will be reported and the program will abort whenever necessary
 * boundary condition information is missing in the input database, or
 * when the data read in is either unknown or inconsistent.  The periodic
 * domain information is used to determine which boundary face, edge, or
 * node entries are not required from input.  Error checking requires
 * that node and edge boundary conditions are consistent with those
 * specified for the faces.
 *
 * Arguments are:
 *    bdry_strategy .... object that reads DIRICHLET or NEUMANN conditions
 *    input_db ......... input database containing all boundary data
 *    face_conds ....... array into which integer boundary conditions
 *                       for faces are read
 *    edge_conds ....... array into which integer boundary conditions
 *                       for edges are read
 *    node_conds ....... array into which integer boundary conditions
 *                       for nodes are read
 *    periodic ......... integer vector specifying which coordinate
 *                       directions are periodic (value returned from
 *                       GridGeometry3::getPeriodicShift())
 */
void
CartesianBoundaryUtilities3::getFromInput(
    BoundaryUtilityStrategy* bdry_strategy,
    const boost::shared_ptr<tbox::Database>& input_db,
    std::vector<int>& face_conds,
    std::vector<int>& edge_conds,
    std::vector<int>& node_conds,
    const hier::IntVector& periodic)
{
    TBOX_DIM_ASSERT(periodic.getDim() == tbox::Dimension(3));
    
    TBOX_ASSERT(bdry_strategy != 0);
    TBOX_ASSERT(static_cast<int>(face_conds.size()) == NUM_3D_FACES);
    TBOX_ASSERT(static_cast<int>(edge_conds.size()) == NUM_3D_EDGES);
    TBOX_ASSERT(static_cast<int>(node_conds.size()) == NUM_3D_NODES);
    
    if (!input_db)
    {
        TBOX_ERROR(
            ": CartesianBoundaryUtility3::getFromInput()\n"
            << "no input database supplied"
            << std::endl);
    }
    
    read3dBdryFaces(
        bdry_strategy,
        input_db,
        face_conds,
        periodic);
    
    read3dBdryEdges(
        input_db,
        face_conds,
        edge_conds,
        periodic);
    
    read3dBdryNodes(
        input_db,
        face_conds,
        node_conds,
        periodic);

}


/*
 * Function to fill face boundary values.
 *
 * Arguments are:
 *    var_name ............. name of variable (for error reporting)
 *    var_data ............. cell-centered patch data object to check
 *    patch ................ patch on which data object lives
 *    bdry_face_conds ...... array of boundary conditions for patch faces
 *    bdry_face_values ..... array of boundary values for faces
 *                           (this must be consistent with boundary
 *                           condition types)
 *    ghost_width_to_fill .. width of ghost region to fill
 */
void
CartesianBoundaryUtilities3::fillFaceBoundaryData(
    const std::string& var_name,
    const boost::shared_ptr<pdat::CellData<double> >& var_data,
    const hier::Patch& patch,
    const std::vector<int>& bdry_face_conds,
    const std::vector<double>& bdry_face_values,
    const hier::IntVector& ghost_width_to_fill)
{
}


/*
 * Function to fill edge boundary values.
 *
 * Arguments are:
 *    var_name ............. name of variable (for error reporting)
 *    var_data ............. cell-centered patch data object to check
 *    patch ................ patch on which data object lives
 *    bdry_edge_conds ...... array of boundary conditions for patch edges
 *    bdry_face_values ..... array of boundary values for faces
 *                           (this must be consistent with boundary
 *                           condition types)
 *    ghost_width_to_fill .. width of ghost region to fill
 */
void
CartesianBoundaryUtilities3::fillEdgeBoundaryData(
    const std::string& var_name,
    const boost::shared_ptr<pdat::CellData<double> >& var_data,
    const hier::Patch& patch,
    const std::vector<int>& bdry_edge_conds,
    const std::vector<double>& bdry_face_values,
    const hier::IntVector& ghost_width_to_fill)
{
}


/*
 * Function to fill node boundary values.
 *
 * Arguments are:
 *    var_name ............. name of variable (for error reporting)
 *    var_data ............. cell-centered patch data object to check
 *    patch ................ patch on which data object lives
 *    bdry_node_conds ...... array of boundary conditions for patch nodes
 *    bdry_face_values ..... array of boundary values for faces
 *                           (this must be consistent with boundary
 *                           condition types)
 *    ghost_width_to_fill .. width of ghost region to fill
 */
void
CartesianBoundaryUtilities3::fillNodeBoundaryData(
    const std::string& var_name,
    const boost::shared_ptr<pdat::CellData<double> >& var_data,
    const hier::Patch& patch,
    const std::vector<int>& bdry_node_conds,
    const std::vector<double>& bdry_face_values,
    const hier::IntVector& ghost_width_to_fill)
{
}


/*
 * Function that returns the integer face boundary location
 * corresponding to the given edge location and edge boundary
 * condition.
 *
 * If the edge boundary condition type or edge location are unknown,
 * or the boundary condition type is inconsistant with the edge location
 * an error results.
 */
int
CartesianBoundaryUtilities3::getFaceLocationForEdgeBdry(
    int edge_loc,
    int edge_btype)
{
    int ret_face = -1;
    
    switch (edge_btype)
    {
        case BdryCond::Basic::XFLOW:
        case BdryCond::Basic::XREFLECT:
        case BdryCond::Basic::XSYMMETRY:
        case BdryCond::Basic::XDIRICHLET:
        case BdryCond::Basic::XNEUMANN:
        {
            if (edge_loc == EdgeBdyLoc3D::XLO_ZLO ||
                edge_loc == EdgeBdyLoc3D::XLO_ZHI ||
                edge_loc == EdgeBdyLoc3D::XLO_YLO ||
                edge_loc == EdgeBdyLoc3D::XLO_YHI)
            {
                ret_face = BdryLoc::XLO;
            }
            else if (edge_loc == EdgeBdyLoc3D::XHI_ZLO ||
                     edge_loc == EdgeBdyLoc3D::XHI_ZHI ||
                     edge_loc == EdgeBdyLoc3D::XHI_YLO ||
                     edge_loc == EdgeBdyLoc3D::XHI_YHI)
            {
                ret_face = BdryLoc::XHI;
            }
            break;
        }
        case BdryCond::Basic::YFLOW:
        case BdryCond::Basic::YREFLECT:
        case BdryCond::Basic::YSYMMETRY:
        case BdryCond::Basic::YDIRICHLET:
        case BdryCond::Basic::YNEUMANN:
        {
            if (edge_loc == EdgeBdyLoc3D::YLO_ZLO ||
                edge_loc == EdgeBdyLoc3D::YLO_ZHI ||
                edge_loc == EdgeBdyLoc3D::XLO_YLO ||
                edge_loc == EdgeBdyLoc3D::XHI_YLO)
            {
                ret_face = BdryLoc::YLO;
            }
            else if (edge_loc == EdgeBdyLoc3D::YHI_ZLO ||
                       edge_loc == EdgeBdyLoc3D::YHI_ZHI ||
                       edge_loc == EdgeBdyLoc3D::XLO_YHI ||
                       edge_loc == EdgeBdyLoc3D::XHI_YHI)
            {
                ret_face = BdryLoc::YHI;
            }
            break;
        }
        case BdryCond::Basic::ZFLOW:
        case BdryCond::Basic::ZREFLECT:
        case BdryCond::Basic::ZSYMMETRY:
        case BdryCond::Basic::ZDIRICHLET:
        case BdryCond::Basic::ZNEUMANN:
        {
           if (edge_loc == EdgeBdyLoc3D::YLO_ZLO ||
               edge_loc == EdgeBdyLoc3D::YHI_ZLO ||
               edge_loc == EdgeBdyLoc3D::XLO_ZLO ||
               edge_loc == EdgeBdyLoc3D::XHI_ZLO)
            {
                ret_face = BdryLoc::ZLO;
            }
            else if (edge_loc == EdgeBdyLoc3D::YLO_ZHI ||
                     edge_loc == EdgeBdyLoc3D::YHI_ZHI ||
                     edge_loc == EdgeBdyLoc3D::XLO_ZHI ||
                     edge_loc == EdgeBdyLoc3D::XHI_ZHI)
            {
                ret_face = BdryLoc::ZHI;
            }
            break;
        }
        default:
        {
            TBOX_ERROR(
                "Unknown edge boundary condition type = "
                << edge_btype << " passed to \n"
                << "CartesianBoundaryUtilities3::getFaceLocationForEdgeBdry"
                << std::endl);
        }
    }
    
    if (ret_face == -1)
    {
        TBOX_ERROR(
            "Edge boundary condition type = "
            << edge_btype << " and edge location = " << edge_loc
            << "\n passed to "
            << "CartesianBoundaryUtilities3::getFaceLocationForEdgeBdry"
            << " are inconsistant." << std::endl);
    }
    
    return ret_face;
}


/*
 * Function that returns the integer face boundary location
 * corresponding to the given node location and node boundary
 * condition.
 *
 * If the node boundary condition type or node location are unknown,
 * or the boundary condition type is inconsistant with the node location
 * an error results.
 */
int
CartesianBoundaryUtilities3::getFaceLocationForNodeBdry(
    int node_loc,
    int node_btype)
{
    int ret_face = -1;
    
    switch (node_btype)
    {
        case BdryCond::Basic::XFLOW:
        case BdryCond::Basic::XREFLECT:
        case BdryCond::Basic::XSYMMETRY:
        case BdryCond::Basic::XDIRICHLET:
        case BdryCond::Basic::XNEUMANN:
        {
            if (node_loc == NodeBdyLoc3D::XLO_YLO_ZLO ||
                node_loc == NodeBdyLoc3D::XLO_YHI_ZLO ||
                node_loc == NodeBdyLoc3D::XLO_YLO_ZHI ||
                node_loc == NodeBdyLoc3D::XLO_YHI_ZHI)
            {
                ret_face = BdryLoc::XLO;
            }
            else
            {
                ret_face = BdryLoc::XHI;
            }
            break;
        }
        case BdryCond::Basic::YFLOW:
        case BdryCond::Basic::YREFLECT:
        case BdryCond::Basic::YSYMMETRY:
        case BdryCond::Basic::YDIRICHLET:
        case BdryCond::Basic::YNEUMANN:
        {
            if (node_loc == NodeBdyLoc3D::XLO_YLO_ZLO ||
                node_loc == NodeBdyLoc3D::XHI_YLO_ZLO ||
                node_loc == NodeBdyLoc3D::XLO_YLO_ZHI ||
                node_loc == NodeBdyLoc3D::XHI_YLO_ZHI)
            {
                ret_face = BdryLoc::YLO;
            }
            else
            {
                ret_face = BdryLoc::YHI;
            }
            break;
        }
        case BdryCond::Basic::ZFLOW:
        case BdryCond::Basic::ZREFLECT:
        case BdryCond::Basic::ZSYMMETRY:
        case BdryCond::Basic::ZDIRICHLET:
        case BdryCond::Basic::ZNEUMANN:
        {
            if (node_loc == NodeBdyLoc3D::XLO_YLO_ZLO ||
                node_loc == NodeBdyLoc3D::XHI_YLO_ZLO ||
                node_loc == NodeBdyLoc3D::XLO_YHI_ZLO ||
                node_loc == NodeBdyLoc3D::XHI_YHI_ZLO)
            {
                ret_face = BdryLoc::ZLO;
            }
            else
            {
                ret_face = BdryLoc::ZHI;
            }
            break;
        }
        default:
        {
            TBOX_ERROR(
                "Unknown node boundary condition type = "
                << node_btype << " passed to \n"
                << "CartesianBoundaryUtilities3::getFaceLocationForNodeBdry"
                << std::endl);
        }
    }
    
    if (ret_face == -1)
    {
        TBOX_ERROR(
            "Node boundary condition type = "
            << node_btype << " and node location = " << node_loc
            << "\n passed to "
            << "CartesianBoundaryUtilities3::getFaceLocationForNodeBdry"
            << " are inconsistant." << std::endl);
    }
    
    return ret_face;
}


/*
 * Private function to read 3D face boundary data from input database.
 */
void
CartesianBoundaryUtilities3::read3dBdryFaces(
    BoundaryUtilityStrategy* bdry_strategy,
    const boost::shared_ptr<tbox::Database>& input_db,
    std::vector<int>& face_conds,
    const hier::IntVector& periodic)
{
    TBOX_DIM_ASSERT(periodic.getDim() == tbox::Dimension(3));
    
    TBOX_ASSERT(bdry_strategy != 0);
    TBOX_ASSERT(input_db);
    TBOX_ASSERT(static_cast<int>(face_conds.size()) == NUM_3D_FACES);
    
    int num_per_dirs = 0;
    for (int id = 0; id < 3; id++)
    {
        if (periodic(id))
            ++num_per_dirs;
    }
    
    if (num_per_dirs < 3)
    {
        // face boundary input required
        for (int s = 0; s < NUM_3D_FACES; s++)
        {
            std::string bdry_loc_str;
            switch (s)
            {
                case BdryLoc::XLO:
                {
                    bdry_loc_str = "boundary_face_xlo";
                    break;
                }
                case BdryLoc::XHI:
                {
                    bdry_loc_str = "boundary_face_xhi";
                    break;
                }
                case BdryLoc::YLO:
                {
                    bdry_loc_str = "boundary_face_ylo";
                    break;
                }
                case BdryLoc::YHI:
                {
                    bdry_loc_str = "boundary_face_yhi";
                    break;
                }
                case BdryLoc::ZLO:
                {
                    bdry_loc_str = "boundary_face_zlo";
                    break;
                }
                case BdryLoc::ZHI:
                {
                    bdry_loc_str = "boundary_face_zhi";
                    break;
                }
                default: NULL_STATEMENT;
            }
            
            bool need_data_read = true;
            if (num_per_dirs > 0)
            {
                if (periodic(0) && (s == BdryLoc::XLO || s == BdryLoc::XHI))
                {
                    need_data_read = false;
                }
                else if (periodic(1) && (s == BdryLoc::YLO || s == BdryLoc::YHI))
                {
                    need_data_read = false;
                }
                else if (periodic(2) && (s == BdryLoc::ZLO || s == BdryLoc::ZHI))
                {
                    need_data_read = false;
                }
            }
            
            if (need_data_read)
            {
                boost::shared_ptr<tbox::Database> bdry_loc_db(
                    input_db->getDatabase(bdry_loc_str));
                std::string bdry_cond_str =
                    bdry_loc_db->getString("boundary_condition");
                if (bdry_cond_str == "FLOW")
                {
                    face_conds[s] = BdryCond::Basic::FLOW;
                }
                else if (bdry_cond_str == "REFLECT")
                {
                    face_conds[s] = BdryCond::Basic::REFLECT;
                }
                else if (bdry_cond_str == "SYMMETRY")
                {
                    face_conds[s] = BdryCond::Basic::SYMMETRY;
                }
                else if (bdry_cond_str == "DIRICHLET")
                {
                    face_conds[s] = BdryCond::Basic::DIRICHLET;
                    bdry_strategy->readDirichletBoundaryDataEntry(
                        bdry_loc_db,
                        bdry_loc_str,
                        s);
                }
                else if (bdry_cond_str == "NEUMANN")
                {
                    face_conds[s] = BdryCond::Basic::NEUMANN;
                    bdry_strategy->readNeumannBoundaryDataEntry(
                        bdry_loc_db,
                        bdry_loc_str,
                        s);
                }
                else
                {
                    TBOX_ERROR(
                        "Unknown face boundary string = "
                        << bdry_cond_str << " found in input." << std::endl);
                }
            } // if (need_data_read)
        } // for (int s = 0 ...
    } // if (num_per_dirs < 3)
}


/*
 * Private function to read 3D edge boundary data from input database.
 */
void
CartesianBoundaryUtilities3::read3dBdryEdges(
    const boost::shared_ptr<tbox::Database>& input_db,
    const std::vector<int>& face_conds,
    std::vector<int>& edge_conds,
    const hier::IntVector& periodic)
{
    TBOX_DIM_ASSERT(periodic.getDim() == tbox::Dimension(3));
    
    TBOX_ASSERT(input_db);
    TBOX_ASSERT(static_cast<int>(face_conds.size()) == NUM_3D_FACES);
    TBOX_ASSERT(static_cast<int>(edge_conds.size()) == NUM_3D_EDGES);
    
    int num_per_dirs = 0;
    for (int id = 0; id < 3; id++)
    {
        if (periodic(id))
            ++num_per_dirs;
    }
    
    if (num_per_dirs < 2)
    {
        // edge boundary input required
        for (int s = 0; s < NUM_3D_EDGES; s++)
        {
            std::string bdry_loc_str;
            switch (s)
            {
                case EdgeBdyLoc3D::YLO_ZLO:
                {
                    bdry_loc_str = "boundary_edge_ylo_zlo";
                    break;
                }
                case EdgeBdyLoc3D::YHI_ZLO:
                {
                    bdry_loc_str = "boundary_edge_yhi_zlo";
                    break;
                }
                case EdgeBdyLoc3D::YLO_ZHI:
                {
                    bdry_loc_str = "boundary_edge_ylo_zhi";
                    break;
                }
                case EdgeBdyLoc3D::YHI_ZHI:
                {
                    bdry_loc_str = "boundary_edge_yhi_zhi";
                    break;
                }
                case EdgeBdyLoc3D::XLO_ZLO:
                {
                    bdry_loc_str = "boundary_edge_xlo_zlo";
                    break;
                }
                case EdgeBdyLoc3D::XLO_ZHI:
                {
                    bdry_loc_str = "boundary_edge_xlo_zhi";
                    break;
                }
                case EdgeBdyLoc3D::XHI_ZLO:
                {
                    bdry_loc_str = "boundary_edge_xhi_zlo";
                    break;
                }
                case EdgeBdyLoc3D::XHI_ZHI:
                {
                    bdry_loc_str = "boundary_edge_xhi_zhi";
                    break;
                }
                case EdgeBdyLoc3D::XLO_YLO:
                {
                    bdry_loc_str = "boundary_edge_xlo_ylo";
                    break;
                }
                case EdgeBdyLoc3D::XHI_YLO:
                {
                    bdry_loc_str = "boundary_edge_xhi_ylo";
                    break;
                }
                case EdgeBdyLoc3D::XLO_YHI:
                {
                    bdry_loc_str = "boundary_edge_xlo_yhi";
                    break;
                }
                case EdgeBdyLoc3D::XHI_YHI:
                {
                    bdry_loc_str = "boundary_edge_xhi_yhi";
                    break;
                }
                default: NULL_STATEMENT;
            }
            
            bool need_data_read = false;
            if (num_per_dirs == 0)
            {
                need_data_read = true;
            }
            else if (periodic(0) &&
                     (s == EdgeBdyLoc3D::YLO_ZLO ||
                      s == EdgeBdyLoc3D::YHI_ZLO ||
                      s == EdgeBdyLoc3D::YLO_ZHI ||
                      s == EdgeBdyLoc3D::YHI_ZHI))
            {
                need_data_read = true;
            }
            else if (periodic(1) &&
                     (s == EdgeBdyLoc3D::XLO_ZLO ||
                      s == EdgeBdyLoc3D::XLO_ZHI ||
                      s == EdgeBdyLoc3D::XHI_ZLO ||
                      s == EdgeBdyLoc3D::XHI_ZHI))
            {
                need_data_read = true;
            }
            else if (periodic(2) &&
                     (s == EdgeBdyLoc3D::XLO_YLO ||
                      s == EdgeBdyLoc3D::XHI_YLO ||
                      s == EdgeBdyLoc3D::XLO_YHI ||
                      s == EdgeBdyLoc3D::XHI_YHI))
            {
                need_data_read = true;
            }
            
            if (need_data_read)
            {
                boost::shared_ptr<tbox::Database> bdry_loc_db(
                   input_db->getDatabase(bdry_loc_str));
                
                std::string bdry_cond_str =
                   bdry_loc_db->getString("boundary_condition");
                if (bdry_cond_str == "XFLOW")
                {
                    edge_conds[s] = BdryCond::Basic::XFLOW;
                }
                else if
                (bdry_cond_str == "YFLOW")
                {
                    edge_conds[s] = BdryCond::Basic::YFLOW;
                }
                else if (bdry_cond_str == "ZFLOW")
                {
                    edge_conds[s] = BdryCond::Basic::ZFLOW;
                }
                else if (bdry_cond_str == "XREFLECT")
                {
                    edge_conds[s] = BdryCond::Basic::XREFLECT;
                }
                else if (bdry_cond_str == "YREFLECT")
                {
                    edge_conds[s] = BdryCond::Basic::YREFLECT;
                }
                else if (bdry_cond_str == "ZREFLECT")
                {
                    edge_conds[s] = BdryCond::Basic::ZREFLECT;
                }
                else if (bdry_cond_str == "XSYMMETRY")
                {
                    edge_conds[s] = BdryCond::Basic::XSYMMETRY;
                }
                else if (bdry_cond_str == "YSYMMETRY")
                {
                    edge_conds[s] = BdryCond::Basic::YSYMMETRY;
                }
                else if (bdry_cond_str == "ZSYMMETRY")
                {
                    edge_conds[s] = BdryCond::Basic::ZSYMMETRY;
                }
                else if (bdry_cond_str == "XDIRICHLET")
                {
                    edge_conds[s] = BdryCond::Basic::XDIRICHLET;
                }
                else if (bdry_cond_str == "YDIRICHLET")
                {
                    edge_conds[s] = BdryCond::Basic::YDIRICHLET;
                }
                else if (bdry_cond_str == "ZDIRICHLET")
                {
                    edge_conds[s] = BdryCond::Basic::ZDIRICHLET;
                }
                else if (bdry_cond_str == "XNEUMANN")
                {
                    edge_conds[s] = BdryCond::Basic::XNEUMANN;
                }
                else if (bdry_cond_str == "YNEUMANN")
                {
                    edge_conds[s] = BdryCond::Basic::YNEUMANN;
                }
                else if (bdry_cond_str == "ZNEUMANN")
                {
                    edge_conds[s] = BdryCond::Basic::ZNEUMANN;
                }
                else
                {
                    TBOX_ERROR(
                        "Unknown edge boundary string = "
                        << bdry_cond_str
                        << " found in input."
                        << std::endl);
                }
                
                bool ambiguous_type = false;
                if (bdry_cond_str == "XFLOW" ||
                    bdry_cond_str == "XREFLECT" ||
                    bdry_cond_str == "XSYMMETRY" ||
                    bdry_cond_str == "XDIRICHLET" ||
                    bdry_cond_str == "XNEUMANN")
                {
                    if (s == EdgeBdyLoc3D::YLO_ZLO ||
                        s == EdgeBdyLoc3D::YHI_ZLO ||
                        s == EdgeBdyLoc3D::YLO_ZHI ||
                        s == EdgeBdyLoc3D::YHI_ZHI)
                    {
                        ambiguous_type = true;
                    }
                }
                else if (bdry_cond_str == "YFLOW" ||
                         bdry_cond_str == "YREFLECT" ||
                         bdry_cond_str == "YSYMMETRY" ||
                         bdry_cond_str == "YDIRICHLET" ||
                         bdry_cond_str == "YNEUMANN")
                {
                    if (s == EdgeBdyLoc3D::XLO_ZLO ||
                        s == EdgeBdyLoc3D::XLO_ZHI ||
                        s == EdgeBdyLoc3D::XHI_ZLO ||
                        s == EdgeBdyLoc3D::XHI_ZHI)
                    {
                        ambiguous_type = true;
                    }
                }
                else if (bdry_cond_str == "ZFLOW" ||
                         bdry_cond_str == "ZREFLECT" ||
                         bdry_cond_str == "ZSYMMETRY" ||
                         bdry_cond_str == "ZDIRICHLET" ||
                         bdry_cond_str == "ZNEUMANN")
                {
                    if (s == EdgeBdyLoc3D::XLO_YLO ||
                        s == EdgeBdyLoc3D::XHI_YLO ||
                        s == EdgeBdyLoc3D::XLO_YHI ||
                        s == EdgeBdyLoc3D::XHI_YHI)
                    {
                        ambiguous_type = true;
                    }
                }
                if (ambiguous_type)
                {
                    TBOX_ERROR(
                        "Ambiguous bdry condition "
                        << bdry_cond_str
                        << " found for "
                        << bdry_loc_str
                        << std::endl);
                }
                
                std::string proper_face;
                std::string proper_face_data;
                bool no_face_data_found = false;
                if (bdry_cond_str == "XFLOW" ||
                    bdry_cond_str == "XDIRICHLET" ||
                    bdry_cond_str == "XNEUMANN" ||
                    bdry_cond_str == "XREFLECT" ||
                    bdry_cond_str == "XSYMMETRY")
                {
                    if (s == EdgeBdyLoc3D::XLO_ZLO ||
                        s == EdgeBdyLoc3D::XLO_ZHI ||
                        s == EdgeBdyLoc3D::XLO_YLO ||
                        s == EdgeBdyLoc3D::XLO_YHI)
                    {
                        proper_face = "XLO";
                        if (bdry_cond_str == "XFLOW" &&
                            face_conds[BdryLoc::XLO] != BdryCond::Basic::FLOW)
                        {
                            no_face_data_found = true;
                            proper_face_data = "FLOW";
                        }
                        if (bdry_cond_str == "XDIRICHLET" &&
                            face_conds[BdryLoc::XLO] != BdryCond::Basic::DIRICHLET)
                        {
                            no_face_data_found = true;
                            proper_face_data = "DIRICHLET";
                        }
                        if (bdry_cond_str == "XNEUMANN" &&
                            face_conds[BdryLoc::XLO] != BdryCond::Basic::NEUMANN)
                        {
                            no_face_data_found = true;
                            proper_face_data = "NEUMANN";
                        }
                        if (bdry_cond_str == "XREFLECT" &&
                            face_conds[BdryLoc::XLO] != BdryCond::Basic::REFLECT)
                        {
                            no_face_data_found = true;
                            proper_face_data = "REFLECT";
                        }
                        if (bdry_cond_str == "XSYMMETRY" &&
                            face_conds[BdryLoc::XLO] != BdryCond::Basic::SYMMETRY)
                        {
                            no_face_data_found = true;
                            proper_face_data = "SYMMETRY";
                        }
                    }
                    else
                    {
                        proper_face = "XHI";
                        if (bdry_cond_str == "XFLOW" &&
                            face_conds[BdryLoc::XHI] != BdryCond::Basic::FLOW)
                        {
                            no_face_data_found = true;
                            proper_face_data = "FLOW";
                        }
                        if (bdry_cond_str == "XDIRICHLET" &&
                            face_conds[BdryLoc::XHI] != BdryCond::Basic::DIRICHLET)
                        {
                            no_face_data_found = true;
                            proper_face_data = "DIRICHLET";
                        }
                        if (bdry_cond_str == "XNEUMANN" &&
                            face_conds[BdryLoc::XHI] != BdryCond::Basic::NEUMANN)
                        {
                            no_face_data_found = true;
                            proper_face_data = "NEUMANN";
                        }
                        if (bdry_cond_str == "XREFLECT" &&
                            face_conds[BdryLoc::XHI] != BdryCond::Basic::REFLECT)
                        {
                            no_face_data_found = true;
                            proper_face_data = "REFLECT";
                        }
                        if (bdry_cond_str == "XSYMMETRY" &&
                            face_conds[BdryLoc::XHI] != BdryCond::Basic::SYMMETRY)
                        {
                            no_face_data_found = true;
                            proper_face_data = "SYMMETRY";
                        }
                    }
                }
                else if (bdry_cond_str == "YFLOW" ||
                         bdry_cond_str == "YDIRICHLET" ||
                         bdry_cond_str == "YNEUMANN" ||
                         bdry_cond_str == "YREFLECT" ||
                         bdry_cond_str == "YSYMMETRY")
                {
                    if (s == EdgeBdyLoc3D::YLO_ZLO ||
                        s == EdgeBdyLoc3D::YLO_ZHI ||
                        s == EdgeBdyLoc3D::XLO_YLO ||
                        s == EdgeBdyLoc3D::XHI_YLO)
                    {
                        proper_face = "YLO";
                        if (bdry_cond_str == "YFLOW" &&
                            face_conds[BdryLoc::YLO] != BdryCond::Basic::FLOW)
                        {
                            no_face_data_found = true;
                            proper_face_data = "FLOW";
                        }
                        if (bdry_cond_str == "YDIRICHLET" &&
                            face_conds[BdryLoc::YLO] != BdryCond::Basic::DIRICHLET)
                        {
                            no_face_data_found = true;
                            proper_face_data = "DIRICHLET";
                        }
                        if (bdry_cond_str == "YNEUMANN" &&
                            face_conds[BdryLoc::YLO] != BdryCond::Basic::NEUMANN)
                        {
                            no_face_data_found = true;
                            proper_face_data = "NEUMANN";
                        }
                        if (bdry_cond_str == "YREFLECT" &&
                            face_conds[BdryLoc::YLO] != BdryCond::Basic::REFLECT)
                        {
                            no_face_data_found = true;
                            proper_face_data = "REFLECT";
                        }
                        if (bdry_cond_str == "YSYMMETRY" &&
                            face_conds[BdryLoc::YLO] != BdryCond::Basic::SYMMETRY)
                        {
                            no_face_data_found = true;
                            proper_face_data = "SYMMETRY";
                        }
                    }
                    else
                    {
                        proper_face = "YHI";
                        if (bdry_cond_str == "YFLOW" &&
                            face_conds[BdryLoc::YHI] != BdryCond::Basic::FLOW)
                        {
                            no_face_data_found = true;
                            proper_face_data = "FLOW";
                        }
                        if (bdry_cond_str == "YDIRICHLET" &&
                            face_conds[BdryLoc::YHI] != BdryCond::Basic::DIRICHLET)
                        {
                            no_face_data_found = true;
                            proper_face_data = "DIRICHLET";
                        }
                        if (bdry_cond_str == "YNEUMANN" &&
                            face_conds[BdryLoc::YHI] != BdryCond::Basic::NEUMANN)
                        {
                            no_face_data_found = true;
                            proper_face_data = "NEUMANN";
                        }
                        if (bdry_cond_str == "YREFLECT" &&
                            face_conds[BdryLoc::YHI] != BdryCond::Basic::REFLECT)
                        {
                            no_face_data_found = true;
                            proper_face_data = "REFLECT";
                        }
                        if (bdry_cond_str == "YSYMMETRY" &&
                            face_conds[BdryLoc::YHI] != BdryCond::Basic::SYMMETRY)
                        {
                            no_face_data_found = true;
                            proper_face_data = "SYMMETRY";
                        }
                    }
                }
                else if (bdry_cond_str == "ZFLOW" ||
                         bdry_cond_str == "ZDIRICHLET" ||
                         bdry_cond_str == "ZNEUMANN" ||
                         bdry_cond_str == "ZREFLECT" ||
                         bdry_cond_str == "ZSYMMETRY")
                {
                    if (s == EdgeBdyLoc3D::XLO_ZLO ||
                        s == EdgeBdyLoc3D::YHI_ZLO ||
                        s == EdgeBdyLoc3D::YLO_ZLO ||
                        s == EdgeBdyLoc3D::XHI_ZLO)
                    {
                        proper_face = "ZLO";
                        if (bdry_cond_str == "ZFLOW" &&
                            face_conds[BdryLoc::ZLO] != BdryCond::Basic::FLOW)
                        {
                            no_face_data_found = true;
                            proper_face_data = "FLOW";
                        }
                        if (bdry_cond_str == "ZDIRICHLET" &&
                            face_conds[BdryLoc::ZLO] != BdryCond::Basic::DIRICHLET)
                        {
                            no_face_data_found = true;
                            proper_face_data = "DIRICHLET";
                        }
                        if (bdry_cond_str == "ZNEUMANN" &&
                            face_conds[BdryLoc::ZLO] != BdryCond::Basic::NEUMANN)
                        {
                            no_face_data_found = true;
                            proper_face_data = "NEUMANN";
                        }
                        if (bdry_cond_str == "ZREFLECT" &&
                            face_conds[BdryLoc::ZLO] != BdryCond::Basic::REFLECT)
                        {
                            no_face_data_found = true;
                            proper_face_data = "REFLECT";
                        }
                        if (bdry_cond_str == "ZSYMMETRY" &&
                            face_conds[BdryLoc::ZLO] != BdryCond::Basic::SYMMETRY)
                        {
                            no_face_data_found = true;
                            proper_face_data = "SYMMETRY";
                        }
                    }
                    else
                    {
                        proper_face = "ZHI";
                        if (bdry_cond_str == "ZFLOW" &&
                            face_conds[BdryLoc::ZHI] != BdryCond::Basic::FLOW)
                        {
                            no_face_data_found = true;
                            proper_face_data = "FLOW";
                        }
                        if (bdry_cond_str == "ZDIRICHLET" &&
                            face_conds[BdryLoc::ZHI] != BdryCond::Basic::DIRICHLET)
                        {
                            no_face_data_found = true;
                            proper_face_data = "DIRICHLET";
                        }
                        if (bdry_cond_str == "ZNEUMANN" &&
                            face_conds[BdryLoc::ZHI] != BdryCond::Basic::NEUMANN)
                        {
                            no_face_data_found = true;
                            proper_face_data = "NEUMANN";
                        }
                        if (bdry_cond_str == "ZREFLECT" &&
                            face_conds[BdryLoc::ZHI] != BdryCond::Basic::REFLECT)
                        {
                            no_face_data_found = true;
                            proper_face_data = "REFLECT";
                        }
                        if (bdry_cond_str == "ZSYMMETRY" &&
                            face_conds[BdryLoc::ZHI] != BdryCond::Basic::SYMMETRY)
                        {
                            no_face_data_found = true;
                            proper_face_data = "SYMMETRY";
                        }
                    }
                }
                if (no_face_data_found)
                {
                    TBOX_ERROR(
                        "Bdry condition "
                        << bdry_cond_str
                        << " found for "
                        << bdry_loc_str
                        << "\n but no "
                        << proper_face_data
                        << " data found for face "
                        << proper_face << std::endl);
                }
            } // if (need_data_read)
        } // for (int s = 0 ...
    } // if (num_per_dirs < 2)
}
    
    
/*
 * Private function to read 3D node boundary data from input database.
 */
void
CartesianBoundaryUtilities3::read3dBdryNodes(
    const boost::shared_ptr<tbox::Database>& input_db,
    const std::vector<int>& face_conds,
    std::vector<int>& node_conds,
    const hier::IntVector& periodic)
{
    TBOX_DIM_ASSERT(periodic.getDim() == tbox::Dimension(3));
    
    TBOX_ASSERT(input_db);
    TBOX_ASSERT(static_cast<int>(face_conds.size()) == NUM_3D_FACES);
    TBOX_ASSERT(static_cast<int>(node_conds.size()) == NUM_3D_NODES);
    
    int num_per_dirs = 0;
    for (int id = 0; id < 3; id++)
    {
        if (periodic(id))
            ++num_per_dirs;
    }
    
    if (num_per_dirs < 1)
    {
        // node boundary data required
        for (int s = 0; s < NUM_3D_NODES; s++)
        {
            std::string bdry_loc_str;
            switch (s)
            {
                case NodeBdyLoc3D::XLO_YLO_ZLO:
                {
                    bdry_loc_str = "boundary_node_xlo_ylo_zlo";
                    break;
                }
                case NodeBdyLoc3D::XHI_YLO_ZLO:
                {
                    bdry_loc_str = "boundary_node_xhi_ylo_zlo";
                    break;
                }
                case NodeBdyLoc3D::XLO_YHI_ZLO:
                {
                    bdry_loc_str = "boundary_node_xlo_yhi_zlo";
                    break;
                }
                case NodeBdyLoc3D::XHI_YHI_ZLO:
                {
                    bdry_loc_str = "boundary_node_xhi_yhi_zlo";
                    break;
                }
                case NodeBdyLoc3D::XLO_YLO_ZHI:
                {
                    bdry_loc_str = "boundary_node_xlo_ylo_zhi";
                    break;
                }
                case NodeBdyLoc3D::XHI_YLO_ZHI:
                {
                    bdry_loc_str = "boundary_node_xhi_ylo_zhi";
                    break;
                }
                case NodeBdyLoc3D::XLO_YHI_ZHI:
                {
                    bdry_loc_str = "boundary_node_xlo_yhi_zhi";
                    break;
                }
                case NodeBdyLoc3D::XHI_YHI_ZHI:
                {
                    bdry_loc_str = "boundary_node_xhi_yhi_zhi";
                    break;
                }
                default: NULL_STATEMENT;
            }
            boost::shared_ptr<tbox::Database> bdry_loc_db(
                input_db->getDatabase(bdry_loc_str));
            std::string bdry_cond_str =
                bdry_loc_db->getString("boundary_condition");
            if (bdry_cond_str == "XFLOW")
            {
                node_conds[s] = BdryCond::Basic::XFLOW;
            }
            else if (bdry_cond_str == "YFLOW")
            {
                node_conds[s] = BdryCond::Basic::YFLOW;
            }
            else if (bdry_cond_str == "ZFLOW")
            {
                node_conds[s] = BdryCond::Basic::ZFLOW;
            }
            else if (bdry_cond_str == "XREFLECT")
            {
                node_conds[s] = BdryCond::Basic::XREFLECT;
            }
            else if (bdry_cond_str == "YREFLECT")
            {
                node_conds[s] = BdryCond::Basic::YREFLECT;
            }
            else if (bdry_cond_str == "ZREFLECT")
            {
                node_conds[s] = BdryCond::Basic::ZREFLECT;
            }
            else if (bdry_cond_str == "XSYMMETRY")
            {
                node_conds[s] = BdryCond::Basic::XSYMMETRY;
            }
            else if (bdry_cond_str == "YSYMMETRY")
            {
                node_conds[s] = BdryCond::Basic::YSYMMETRY;
            }
            else if (bdry_cond_str == "ZSYMMETRY")
            {
                node_conds[s] = BdryCond::Basic::ZSYMMETRY;
            }
            else if (bdry_cond_str == "XDIRICHLET")
            {
                node_conds[s] = BdryCond::Basic::XDIRICHLET;
            }
            else if (bdry_cond_str == "YDIRICHLET")
            {
                node_conds[s] = BdryCond::Basic::YDIRICHLET;
            }
            else if (bdry_cond_str == "ZDIRICHLET")
            {
                node_conds[s] = BdryCond::Basic::ZDIRICHLET;
            }
            else if (bdry_cond_str == "XNEUMANN")
            {
                node_conds[s] = BdryCond::Basic::XNEUMANN;
            }
            else if (bdry_cond_str == "YNEUMANN")
            {
                node_conds[s] = BdryCond::Basic::YNEUMANN;
            }
            else if (bdry_cond_str == "ZNEUMANN")
            {
                node_conds[s] = BdryCond::Basic::ZNEUMANN;
            }
            else
            {
                TBOX_ERROR(
                    "Unknown node boundary string = "
                    << bdry_cond_str
                    << " found in input."
                    << std::endl);
            }
            
            std::string proper_face;
            std::string proper_face_data;
            bool no_face_data_found = false;
            if (bdry_cond_str == "XFLOW" ||
                bdry_cond_str == "XDIRICHLET" ||
                bdry_cond_str == "XNEUMANN" ||
                bdry_cond_str == "XREFLECT" ||
                bdry_cond_str == "XSYMMETRY")
            {
                if (s == NodeBdyLoc3D::XLO_YLO_ZLO ||
                    s == NodeBdyLoc3D::XLO_YHI_ZLO ||
                    s == NodeBdyLoc3D::XLO_YLO_ZHI ||
                    s == NodeBdyLoc3D::XLO_YHI_ZHI)
                {
                    proper_face = "XLO";
                    if (bdry_cond_str == "XFLOW" &&
                        face_conds[BdryLoc::XLO] != BdryCond::Basic::FLOW)
                    {
                        no_face_data_found = true;
                        proper_face_data = "FLOW";
                    }
                    if (bdry_cond_str == "XDIRICHLET" &&
                        face_conds[BdryLoc::XLO] != BdryCond::Basic::DIRICHLET)
                    {
                        no_face_data_found = true;
                        proper_face_data = "DIRICHLET";
                    }
                    if (bdry_cond_str == "XNEUMANN" &&
                        face_conds[BdryLoc::XLO] != BdryCond::Basic::NEUMANN)
                    {
                        no_face_data_found = true;
                        proper_face_data = "NEUMANN";
                    }
                    if (bdry_cond_str == "XREFLECT" &&
                        face_conds[BdryLoc::XLO] != BdryCond::Basic::REFLECT)
                    {
                        no_face_data_found = true;
                        proper_face_data = "REFLECT";
                    }
                    if (bdry_cond_str == "XSYMMETRY" &&
                        face_conds[BdryLoc::XLO] != BdryCond::Basic::SYMMETRY)
                    {
                        no_face_data_found = true;
                        proper_face_data = "SYMMETRY";
                    }
                }
                else
                {
                    proper_face = "XHI";
                    if (bdry_cond_str == "XFLOW" &&
                        face_conds[BdryLoc::XHI] != BdryCond::Basic::FLOW)
                    {
                        no_face_data_found = true;
                        proper_face_data = "FLOW";
                    }
                    if (bdry_cond_str == "XDIRICHLET" &&
                        face_conds[BdryLoc::XHI] != BdryCond::Basic::DIRICHLET)
                    {
                        no_face_data_found = true;
                        proper_face_data = "DIRICHLET";
                    }
                    if (bdry_cond_str == "XNEUMANN" &&
                        face_conds[BdryLoc::XHI] != BdryCond::Basic::NEUMANN)
                    {
                        no_face_data_found = true;
                        proper_face_data = "NEUMANN";
                    }
                    if (bdry_cond_str == "XREFLECT" &&
                        face_conds[BdryLoc::XHI] != BdryCond::Basic::REFLECT)
                    {
                       no_face_data_found = true;
                       proper_face_data = "REFLECT";
                    }
                    if (bdry_cond_str == "XSYMMETRY" &&
                        face_conds[BdryLoc::XHI] != BdryCond::Basic::SYMMETRY)
                    {
                       no_face_data_found = true;
                       proper_face_data = "SYMMETRY";
                    }
                }
            }
            else if (bdry_cond_str == "YFLOW" ||
                     bdry_cond_str == "YDIRICHLET" ||
                     bdry_cond_str == "YNEUMANN" ||
                     bdry_cond_str == "YREFLECT" ||
                     bdry_cond_str == "YSYMMETRY")
            {
                if (s == NodeBdyLoc3D::XLO_YLO_ZLO ||
                    s == NodeBdyLoc3D::XHI_YLO_ZLO ||
                    s == NodeBdyLoc3D::XLO_YLO_ZHI ||
                    s == NodeBdyLoc3D::XHI_YLO_ZHI)
                {
                    proper_face = "YLO";
                    if (bdry_cond_str == "YFLOW" &&
                        face_conds[BdryLoc::YLO] != BdryCond::Basic::FLOW)
                    {
                        no_face_data_found = true;
                        proper_face_data = "FLOW";
                    }
                    if (bdry_cond_str == "YDIRICHLET" &&
                        face_conds[BdryLoc::YLO] != BdryCond::Basic::DIRICHLET)
                    {
                        no_face_data_found = true;
                        proper_face_data = "DIRICHLET";
                    }
                    if (bdry_cond_str == "YNEUMANN" &&
                        face_conds[BdryLoc::YLO] != BdryCond::Basic::NEUMANN)
                    {
                        no_face_data_found = true;
                        proper_face_data = "NEUMANN";
                    }
                    if (bdry_cond_str == "YREFLECT" &&
                        face_conds[BdryLoc::YLO] != BdryCond::Basic::REFLECT)
                    {
                        no_face_data_found = true;
                        proper_face_data = "REFLECT";
                    }
                    if (bdry_cond_str == "YSYMMETRY" &&
                        face_conds[BdryLoc::YLO] != BdryCond::Basic::SYMMETRY)
                    {
                        no_face_data_found = true;
                        proper_face_data = "SYMMETRY";
                    }
                }
                else
                {
                    proper_face = "YHI";
                    if (bdry_cond_str == "YFLOW" &&
                        face_conds[BdryLoc::YHI] != BdryCond::Basic::FLOW)
                    {
                        no_face_data_found = true;
                        proper_face_data = "FLOW";
                    }
                    if (bdry_cond_str == "YDIRICHLET" &&
                        face_conds[BdryLoc::YHI] != BdryCond::Basic::DIRICHLET)
                    {
                        no_face_data_found = true;
                        proper_face_data = "DIRICHLET";
                    }
                    if (bdry_cond_str == "YNEUMANN" &&
                        face_conds[BdryLoc::YHI] != BdryCond::Basic::NEUMANN)
                    {
                        no_face_data_found = true;
                        proper_face_data = "NEUMANN";
                    }
                    if (bdry_cond_str == "YREFLECT" &&
                        face_conds[BdryLoc::YHI] != BdryCond::Basic::REFLECT)
                    {
                        no_face_data_found = true;
                        proper_face_data = "REFLECT";
                    }
                    if (bdry_cond_str == "YSYMMETRY" &&
                        face_conds[BdryLoc::YHI] != BdryCond::Basic::SYMMETRY)
                    {
                        no_face_data_found = true;
                        proper_face_data = "SYMMETRY";
                    }
                }
            }
            else if (bdry_cond_str == "ZFLOW" ||
                     bdry_cond_str == "ZDIRICHLET" ||
                     bdry_cond_str == "ZNEUMANN" ||
                     bdry_cond_str == "ZREFLECT" ||
                     bdry_cond_str == "ZSYMMETRY")
            {
                if (s == NodeBdyLoc3D::XLO_YLO_ZLO ||
                    s == NodeBdyLoc3D::XHI_YLO_ZLO ||
                    s == NodeBdyLoc3D::XLO_YHI_ZLO ||
                    s == NodeBdyLoc3D::XHI_YHI_ZLO)
                {
                    proper_face = "ZLO";
                    if (bdry_cond_str == "ZFLOW" &&
                        face_conds[BdryLoc::ZLO] != BdryCond::Basic::FLOW) {
                        no_face_data_found = true;
                        proper_face_data = "FLOW";
                    }
                    if (bdry_cond_str == "ZDIRICHLET" &&
                        face_conds[BdryLoc::ZLO] != BdryCond::Basic::DIRICHLET) {
                        no_face_data_found = true;
                        proper_face_data = "DIRICHLET";
                    }
                    if (bdry_cond_str == "ZNEUMANN" &&
                        face_conds[BdryLoc::ZLO] != BdryCond::Basic::NEUMANN) {
                        no_face_data_found = true;
                        proper_face_data = "NEUMANN";
                    }
                    if (bdry_cond_str == "ZREFLECT" &&
                        face_conds[BdryLoc::ZLO] != BdryCond::Basic::REFLECT) {
                        no_face_data_found = true;
                        proper_face_data = "REFLECT";
                    }
                    if (bdry_cond_str == "ZSYMMETRY" &&
                        face_conds[BdryLoc::ZLO] != BdryCond::Basic::SYMMETRY) {
                        no_face_data_found = true;
                        proper_face_data = "SYMMETRY";
                    }
                }
                else
                {
                    proper_face = "ZHI";
                    if (bdry_cond_str == "ZFLOW" &&
                        face_conds[BdryLoc::ZHI] != BdryCond::Basic::FLOW)
                    {
                        no_face_data_found = true;
                        proper_face_data = "FLOW";
                    }
                    if (bdry_cond_str == "ZDIRICHLET" &&
                        face_conds[BdryLoc::ZHI] != BdryCond::Basic::DIRICHLET)
                    {
                        no_face_data_found = true;
                        proper_face_data = "DIRICHLET";
                    }
                    if (bdry_cond_str == "ZNEUMANN" &&
                        face_conds[BdryLoc::ZHI] != BdryCond::Basic::NEUMANN)
                    {
                        no_face_data_found = true;
                        proper_face_data = "NEUMANN";
                    }
                    if (bdry_cond_str == "ZREFLECT" &&
                        face_conds[BdryLoc::ZHI] != BdryCond::Basic::REFLECT)
                    {
                        no_face_data_found = true;
                        proper_face_data = "REFLECT";
                    }
                    if (bdry_cond_str == "ZSYMMETRY" &&
                        face_conds[BdryLoc::ZHI] != BdryCond::Basic::SYMMETRY)
                    {
                        no_face_data_found = true;
                        proper_face_data = "SYMMETRY";
                    }
                }
            }
            if (no_face_data_found)
            {
                TBOX_ERROR(
                    "Bdry condition "
                    << bdry_cond_str
                    << " found for "
                    << bdry_loc_str
                    << "\n but no "
                    << proper_face_data
                    << " data found for face "
                    << proper_face << std::endl);
            }
       } // for (int s = 0 ...
    } // if (num_per_dirs < 1)
}
