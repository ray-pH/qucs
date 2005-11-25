/*
 * digital.cpp - digital base class implementation
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
 * $Id: digital.cpp,v 1.2 2005-11-25 08:27:05 raimi Exp $
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
#include "circuit.h"
#include "constants.h"
#include "digital.h"

#define NODE_OUT 0 /* first node is output node */
#define NODE_IN1 1 /* input nodes start here */

// Constructor.
digital::digital () : circuit () {
  setVoltageSources (1);
  g = NULL;
  Vout = 0;
}

// Destructor.
digital::~digital () {
  freeDigital ();
}

// Reserve space for derivatives.
void digital::initDigital (void) {
  g = (nr_double_t *) malloc ((getSize () - 1) * sizeof (nr_double_t));
}

// Free space of derivatives if necessary.
void digital::freeDigital (void) {
  if (g != NULL) {
    free (g);
    g = NULL;
  }
}

// Returns voltage at given input node.
nr_double_t digital::getVin (int input) {
  return real (getV (NODE_IN1 + input));
}

// Computes the transfer function for the given input node.
nr_double_t digital::calcTransfer (int input) {
  nr_double_t v = getPropertyDouble ("V");
  return tanh (10 * (getVin (input) / v - 0.5));
}

// Computes the transfer functions derivative for the given input node.
nr_double_t digital::calcDerivative (int input) {
  nr_double_t v = getPropertyDouble ("V");
  nr_double_t x = tanh (10 * (getVin (input) / v - 0.5));
  return 10 * (1 + x * x);
}

// Setup constant S-parameter entries.
void digital::initSP (void) {
  initDigital ();
  allocMatrixS ();
  setS (NODE_OUT, NODE_OUT, -1);
  for (i = 0; i < getSize () - 1; i++) {
    setS (NODE_IN1 + i, NODE_IN1 + i, +1);
  }
}

// Setup frequency dependent S-parameter entries.
void digital::calcSP (nr_double_t frequency) {
  nr_double_t t = getPropertyDouble ("t");
  for (i = 0; i < getSize () - 1; i++) {
    setS (NODE_OUT, NODE_IN1 + i,
	  4.0 * polar (g[i], - 2.0 * M_PI * frequency * t));
  }
}

// Initialize constant MNA entries for DC analysis.
void digital::initDC (void) {
  initDigital ();
  allocMatrixMNA ();
  setB (NODE_OUT, VSRC_1, +1);
  setC (VSRC_1, NODE_OUT, -1);
  setE (VSRC_1, 0);
}

// Computes variable MNA entries during DC analysis.
void digital::calcDC (void) {
  calcOutput ();
  calcDerivatives ();
  for (i = 0, Veq = 0; i < getSize () - 1; i++) {
    setC (VSRC_1, NODE_IN1 + i, g[i]);
    Veq += g[i] * getVin (i);
  }
  setE (VSRC_1, Veq - Vout);
}

// Initialize constant MNA entries for AC analysis.
void digital::initAC (void) {
  initDC ();
}

// Computes frequency dependent MNA entries during AC analysis.
void digital::calcAC (nr_double_t frequency) {
  nr_double_t t = getPropertyDouble ("t");
  for (i = 0; i < getSize () - 1; i++) {
    setC (VSRC_1, NODE_IN1 + i, polar (g[i], - 2.0 * M_PI * frequency * t));
  }
}

// Initialize transient analysis.
void digital::initTR (void) {
  initDC ();
}

// Computes MNA entries during transient analysis.
void digital::calcTR (nr_double_t) {
  calcDC ();
}
