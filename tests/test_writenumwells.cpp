
/*
  Copyright 2014 Statoil IT
  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "config.h"

#if HAVE_DYNAMIC_BOOST_TEST
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE EclipseWriter
#include <boost/test/unit_test.hpp>

#include <opm/core/io/eclipse/EclipseWriter.hpp>
#include <opm/core/grid/GridManager.hpp>
#include <opm/core/props/phaseUsageFromDeck.hpp>
#include <opm/core/simulator/BlackoilState.hpp>
#include <opm/core/simulator/WellState.hpp>
#include <opm/core/simulator/SimulatorTimer.hpp>
#include <opm/core/utility/parameters/ParameterGroup.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>

// ERT stuff
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/util/test_work_area.h>

#include <string.h>




void verifyIwelData(const std::string&  filename, int timestep, int numWellsReadFromScheduleFile) {
  ecl_file_type * restart_file = ecl_file_open(filename.c_str(), 0);
  ecl_file_load_all(restart_file);

  ecl_kw_type * intehead_kw = ecl_file_iget_named_kw(restart_file, INTEHEAD_KW, timestep);
  int numwells = ecl_kw_iget_int(intehead_kw, INTEHEAD_NWELLS_INDEX);
  BOOST_ASSERT(numWellsReadFromScheduleFile == numwells);

  int NIWELZ   = ecl_kw_iget_int(intehead_kw, INTEHEAD_NIWELZ_INDEX);
  ecl_kw_type * iwel_kw = ecl_file_iget_named_kw(restart_file, IWEL_KW, timestep);
  int iwel_kw_size = ecl_kw_get_size(iwel_kw);
  int expected_IWEL_data_size = numwells * NIWELZ;
  BOOST_ASSERT(expected_IWEL_data_size == iwel_kw_size);

  ecl_file_close(restart_file);
}


void verifyZwelData(const std::string&  filename, int timestep, int numWellsReadFromScheduleFile) {
  ecl_file_type * restart_file = ecl_file_open(filename.c_str(), 0);
  ecl_file_load_all(restart_file);

  ecl_kw_type * intehead_kw = ecl_file_iget_named_kw(restart_file, INTEHEAD_KW, timestep);
  int numwells = ecl_kw_iget_int(intehead_kw, INTEHEAD_NWELLS_INDEX);
  BOOST_ASSERT(numWellsReadFromScheduleFile == numwells);

  int NZWELZ   = ecl_kw_iget_int(intehead_kw, INTEHEAD_NZWELZ_INDEX);
  ecl_kw_type * zwel_kw = ecl_file_iget_named_kw(restart_file, ZWEL_KW, timestep);
  int zwel_kw_size = ecl_kw_get_size(zwel_kw);

  int expected_ZWEL_data_size = numwells * NZWELZ;
  BOOST_ASSERT(expected_ZWEL_data_size == zwel_kw_size);


  ecl_file_close(restart_file);
}




void  verifyIconData(const std::string&  filename, int timestep) {
  ecl_file_type * restart_file = ecl_file_open(filename.c_str(), 0);
  ecl_file_load_all(restart_file);

  ecl_kw_type * intehead_kw = ecl_file_iget_named_kw(restart_file, INTEHEAD_KW, timestep);
  int numwells = ecl_kw_iget_int(intehead_kw, INTEHEAD_NWELLS_INDEX); //Num wells
  int NICONZ   = ecl_kw_iget_int(intehead_kw, INTEHEAD_NICONZ_INDEX); //Num elements per connection element
  int NCWMAX   = ecl_kw_iget_int(intehead_kw, INTEHEAD_NCWMAX_INDEX); //Max number of connections for a well

  int expected_ICON_data_size = NICONZ * NCWMAX * numwells;

  ecl_kw_type * icon_kw = ecl_file_iget_named_kw(restart_file, ICON_KW, timestep);
  int icon_kw_size = ecl_kw_get_size(icon_kw);

  BOOST_ASSERT(expected_ICON_data_size == icon_kw_size);

  ecl_file_close(restart_file);
}



BOOST_AUTO_TEST_CASE(EclipseWriteNumWells)
{
    std::string eclipse_data_filename    = "testBlackoilState3.DATA";
    std::string eclipse_restart_filename = "TESTBLACKOILSTATE3.UNRST";

    test_work_area_type * test_area = test_work_area_alloc("TEST_EclipseWriteNumWells");
    test_work_area_copy_file(test_area, eclipse_data_filename.c_str());
    test_work_area_set_store(test_area, true);

    Opm::ParserPtr parser(new Opm::Parser());
    Opm::ParserLogPtr parserLog(new Opm::ParserLog);
    Opm::DeckConstPtr deck = parser->parseFile(eclipse_data_filename, true, parserLog);

    std::shared_ptr<Opm::EclipseState> eclipseState(new Opm::EclipseState(deck));

    Opm::parameter::ParameterGroup params;
    params.insertParameter("deck_filename", eclipse_data_filename);

    const Opm::PhaseUsage phaseUsage = Opm::phaseUsageFromDeck(deck);

    std::string output_dir(test_work_area_get_cwd(test_area));

    std::shared_ptr<Opm::EclipseWriter> eclWriter(new Opm::EclipseWriter(params,
                                                                         eclipseState,
                                                                         phaseUsage,
                                                                         eclipseState->getEclipseGrid()->getCartesianSize(),
                                                                         0));

    std::shared_ptr<Opm::SimulatorTimer> simTimer( new Opm::SimulatorTimer() );
    simTimer->init(eclipseState->getSchedule()->getTimeMap());

    auto eclGrid = eclipseState->getEclipseGrid();

    Opm::EclipseGridConstPtr constEclGrid(eclGrid);
    std::shared_ptr<Opm::GridManager> ourFineGridManagerPtr(new Opm::GridManager(constEclGrid));

    eclWriter->writeInit(*simTimer);

    int countTimeStep = eclipseState->getSchedule()->getTimeMap()->numTimesteps();

    std::shared_ptr<Opm::BlackoilState> blackoilState(new Opm::BlackoilState);
    blackoilState->init(*ourFineGridManagerPtr->c_grid(), 3);
    std::shared_ptr<Opm::WellState> wellState(new Opm::WellState());
    wellState->init(0, *blackoilState);

    for(int i=0; i <= countTimeStep; ++i){
      simTimer->setCurrentStepNum(i);
      eclWriter->writeTimeStep(*simTimer, *blackoilState, *wellState);
    }

    for (size_t timestep = 0; timestep <= eclipseState->getSchedule()->getTimeMap()->numTimesteps(); ++timestep) {
      int numWellsReadFromScheduleFile  = eclipseState->getSchedule()->numWells(timestep);
      verifyIwelData(eclipse_restart_filename, timestep, numWellsReadFromScheduleFile);
      verifyZwelData(eclipse_restart_filename, timestep, numWellsReadFromScheduleFile);
      verifyIconData(eclipse_restart_filename, timestep);
    }
    test_work_area_free(test_area);
}
