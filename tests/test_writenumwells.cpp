
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
#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/fortio.h>


#include <string.h>
#include <memory>

std::shared_ptr<Opm::EclipseWriter> eclWriter;
std::shared_ptr<Opm::SimulatorTimer> simTimer;
std::shared_ptr<const Opm::Deck> deck;
std::shared_ptr<Opm::EclipseState> eclipseState;
std::shared_ptr<Opm::GridManager> ourFineGridManagerPtr;
std::shared_ptr<Opm::BlackoilState> blackoilState;
std::shared_ptr<Opm::WellState> wellState;

void createEclipseWriter(const char *deckString)
{

}

void readDataFile(){

}

BOOST_AUTO_TEST_CASE(EclipseWriteNumWells)
{
    Opm::ParserPtr parser(new Opm::Parser());
    std::string file = "testBlackoilState3.DATA";
    Opm::ParserLogPtr parserLog(new Opm::ParserLog);
    Opm::DeckConstPtr deck = parser->parseFile(file, true, parserLog);
    Opm::Schedule sched( deck );
    std::cerr << "Wells: " << sched.numWells() << std::endl;
    BOOST_ASSERT(deck!=NULL);

    //createEclipseWriter(deckString);

    //test
    //eclWriter->writeInit(*simTimer);


}
