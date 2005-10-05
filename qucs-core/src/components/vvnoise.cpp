/*
 * vvnoise.cpp - correlated noise voltage sources class implementation
 *
 * Copyright (C) 2005 Stefan Jahn <stefan@lkcc.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this package; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
 * Boston, MA 02110-1301, USA.  
 *
 * $Id: vvnoise.cpp,v 1.1 2005-10-05 11:22:42 raimi Exp $
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "complex.h"
#include "object.h"
#include "node.h"
#include "circuit.h"
#include "constants.h"
#include "matrix.h"
#include "component_id.h"
#include "vvnoise.h"

#define NODE_V1P 0
#define NODE_V1N 3
#define NODE_V2P 1
#define NODE_V2N 2

vvnoise::vvnoise () : circuit (4) {
  type = CIR_VVNOISE;
  setVoltageSources (2);
}

void vvnoise::initSP (void) {
  allocMatrixS ();
  setS (NODE_V1P, NODE_V1N, 1.0);
  setS (NODE_V1N, NODE_V1P, 1.0);
  setS (NODE_V2P, NODE_V2N, 1.0);
  setS (NODE_V2N, NODE_V2P, 1.0);
}

void vvnoise::calcNoiseSP (nr_double_t frequency) {
  nr_double_t C = getPropertyDouble ("C");
  nr_double_t e = getPropertyDouble ("e");
  nr_double_t c = getPropertyDouble ("c");
  nr_double_t a = getPropertyDouble ("a");
  nr_double_t k = a + c * pow (frequency, e);
  nr_double_t u1 = getPropertyDouble ("v1") / k / kB / T0 / 4 / z0;
  nr_double_t u2 = getPropertyDouble ("v2") / k / kB / T0 / 4 / z0;
  nr_double_t cu = C * sqrt (u1 * u2);

  // entries of source 1
  setN (NODE_V1P, NODE_V1P, +u1); setN (NODE_V1N, NODE_V1N, +u1);
  setN (NODE_V1P, NODE_V1N, -u1); setN (NODE_V1N, NODE_V1P, -u1);
  // entries of source 2
  setN (NODE_V2P, NODE_V2P, +u2); setN (NODE_V2N, NODE_V2N, +u2);
  setN (NODE_V2P, NODE_V2N, -u2); setN (NODE_V2N, NODE_V2P, -u2);
  // correlation entries
  setN (NODE_V1P, NODE_V2P, +cu); setN (NODE_V1N, NODE_V2N, +cu);
  setN (NODE_V1P, NODE_V2N, -cu); setN (NODE_V1N, NODE_V2P, -cu);
  setN (NODE_V2P, NODE_V1P, +cu); setN (NODE_V2N, NODE_V1N, +cu);
  setN (NODE_V2P, NODE_V1N, -cu); setN (NODE_V2N, NODE_V1P, -cu);
}

void vvnoise::initDC (void) {
  allocMatrixMNA ();
  voltageSource (VSRC_1, NODE_V1P, NODE_V1N);
  voltageSource (VSRC_2, NODE_V2P, NODE_V2N);
}

void vvnoise::initTR (void) {
  initDC ();
}

void vvnoise::initAC (void) {
  initDC ();
}

void vvnoise::calcNoiseAC (nr_double_t frequency) {
  nr_double_t C = getPropertyDouble ("C");
  nr_double_t e = getPropertyDouble ("e");
  nr_double_t c = getPropertyDouble ("c");
  nr_double_t a = getPropertyDouble ("a");
  nr_double_t k = a + c * pow (frequency, e);
  nr_double_t u1 = getPropertyDouble ("v1") / k / kB / T0;
  nr_double_t u2 = getPropertyDouble ("v2") / k / kB / T0;
  nr_double_t cu = C * sqrt (u1 * u2);
  setN (NODE_5, NODE_5, u1); setN (NODE_6, NODE_6, u2);
  setN (NODE_5, NODE_6, cu); setN (NODE_6, NODE_5, cu);
}
