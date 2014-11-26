
/*
  Copyright 2014 Andreas Lauser

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

#include <string.h>
#include <memory>



int readNumWellsFromRestartFile(const std::string&  filename, int timestep) {
  ecl_file_type * restart_file = ecl_file_open(filename.c_str(), 0);
  ecl_file_load_all(restart_file);

  ecl_kw_type * intehead_kw = ecl_file_iget_named_kw(restart_file, INTEHEAD_KW, timestep);
  int numwells = ecl_kw_iget_int(intehead_kw, INTEHEAD_NWELLS_INDEX);

  ecl_file_close(restart_file);

  return numwells;
}



BOOST_AUTO_TEST_CASE(EclipseWriteNumWells)
{
    std::string file = "testBlackoilState4.DATA";
    std::string restart_file = "TESTBLACKOILSTATE4.UNRST";


    Opm::ParserPtr parser(new Opm::Parser());
    Opm::ParserLogPtr parserLog(new Opm::ParserLog);
    Opm::DeckConstPtr deck = parser->parseFile(file, true, parserLog);

    std::shared_ptr<Opm::EclipseState> eclipseState(new Opm::EclipseState(deck));

    Opm::parameter::ParameterGroup params;
    params.insertParameter("deck_filename", file);

    const Opm::PhaseUsage phaseUsage = Opm::phaseUsageFromDeck(deck);

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

    std::cout << "time map " << eclipseState->getSchedule()->getTimeMap()->numTimesteps() << std::endl;

    std::cout << "Numsteps " << simTimer->numSteps() << std::endl;

    int count = 0;

    int countTimeStep = eclipseState->getSchedule()->getTimeMap()->numTimesteps();

    std::shared_ptr<Opm::BlackoilState> blackoilState(new Opm::BlackoilState);
    blackoilState->init(*ourFineGridManagerPtr->c_grid(), 3);
    std::shared_ptr<Opm::WellState> wellState(new Opm::WellState());
    wellState->init(0, *blackoilState);

    for(int i=0; i <= countTimeStep; ++i){
      simTimer->setCurrentStepNum(i);
      eclWriter->writeTimeStep(*simTimer, *blackoilState, *wellState);
    }

    /*
    for (; simTimer->currentStepNum() < simTimer->numSteps(); ++ (*simTimer)) {

        eclWriter->writeTimeStep(*simTimer, *blackoilState, *wellState);
       /* if(count==3){
            eclWriter->writeTimeStep(*simTimer, *blackoilState, *wellState);
          }
    }*/


//    size_t timestep = 0;
//    int numWellsReadFromScheduleFile = eclipseState->getSchedule()->numWells(timestep);
  //  BOOST_ASSERT(numWellsReadFromScheduleFile == readNumWellsFromRestartFile(restart_file, timestep));
}
