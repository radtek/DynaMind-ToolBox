/**
 * @file
 * @author  Christian Mikovits <christian.mikovits@uibk.ac.at>
 * @version 0.1a
 * @section LICENSE
 * Module for controling the urban development cycle
   Copyright (C) 2013 Christian Mikovits

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**/

#ifndef urbandevelTrigger_H
#define urbandevelTrigger_H

#include <dmmodule.h>

class urbandevelTrigger: public DM::Module
{
    DM_DECLARE_NODE(urbandevelTrigger)
public:
    urbandevelTrigger();
    ~urbandevelTrigger();

    void run();
    void init();
private:
    int yearcycle;
    DM::View city;
};

#endif // urbandevelTrigger_H
