
/*========================================================================*
 *                                                                        *
 *  Distributed by Whiteley Research Inc., Sunnyvale, California, USA     *
 *                       http://wrcad.com                                 *
 *  Copyright (C) 2017 Whiteley Research Inc., all rights reserved.       *
 *  Author: Stephen R. Whiteley, except as indicated.                     *
 *                                                                        *
 *  As fully as possible recognizing licensing terms and conditions       *
 *  imposed by earlier work from which this work was derived, if any,     *
 *  this work is released under the Apache License, Version 2.0 (the      *
 *  "License").  You may not use this file except in compliance with      *
 *  the License, and compliance with inherited licenses which are         *
 *  specified in a sub-header below this one if applicable.  A copy       *
 *  of the License is provided with this distribution, or you may         *
 *  obtain a copy of the License at                                       *
 *                                                                        *
 *        http://www.apache.org/licenses/LICENSE-2.0                      *
 *                                                                        *
 *  See the License for the specific language governing permissions       *
 *  and limitations under the License.                                    *
 *                                                                        *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,      *
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES      *
 *   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-        *
 *   INFRINGEMENT.  IN NO EVENT SHALL WHITELEY RESEARCH INCORPORATED      *
 *   OR STEPHEN R. WHITELEY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER     *
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,      *
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE       *
 *   USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                        *
 *========================================================================*
 *               XicTools Integrated Circuit Design System                *
 *                                                                        *
 * WRspice Circuit Simulation and Analysis Tool:  Device Library          *
 *                                                                        *
 *========================================================================*
 $Id:$
 *========================================================================*/

/***************************************************************************
JSPICE3 adaptation of Spice3f2 - Copyright (c) Stephen R. Whiteley 1992
Copyright 1990 Regents of the University of California.  All rights reserved.
Authors: 1985 Hong J. Park, Thomas L. Quarles 
         1993 Stephen R. Whiteley
****************************************************************************/

#include "b1defs.h"


// This routine evaluates the drain current, its derivatives and the
// charges associated with the gate,bulk and drain terminal
// using the BSIM1 (Berkeley Short-Channel IGFET Model) Equations.
//
void
B1dev::evaluate(double vds, double vbs, double vgs, sB1instance *inst,
    sB1model *model, double *gmPointer, double *gdsPointer,
    double *gmbsPointer, double *qgPointer, double *qbPointer,
    double *qdPointer, double *cggbPointer, double *cgdbPointer,
    double *cgsbPointer, double *cbgbPointer, double *cbdbPointer,
    double *cbsbPointer, double *cdgbPointer, double *cddbPointer,
    double *cdsbPointer, double *cdrainPointer, double *vonPointer,
    double *vdsatPointer, sCKT *ckt)
{
    double gm;
    double gds;
    double gmbs;
    double qg;
    double qb;
    double qd;
    double cggb;
    double cgdb;
    double cgsb;
    double cbgb;
    double cbdb;
    double cbsb;
    double cdgb;
    double cddb;
    double cdsb;
    double Vfb;
    double Phi;
    double K1;
    double K2;
    double Vdd;
    double Ugs;
    double Uds;
    double dUgsdVbs;
    double Leff;
    double dUdsdVbs;
    double dUdsdVds;
    double Eta;
    double dEtadVds;
    double dEtadVbs;
    double Vpb;
    double SqrtVpb;
    double Von;
    double Vth;
    double dVthdVbs;
    double dVthdVds;
    double Vgs_Vth;
    double DrainCurrent;
    double G;
    double A;
    double Arg;
    double dGdVbs;
    double dAdVbs;
    double Beta;
    double Beta_Vds_0;
    double BetaVdd;
    double dBetaVdd_dVds;
    double Beta0;
    double dBeta0dVds;
    double dBeta0dVbs;
    double VddSquare;
    double C1;
    double C2;
    double dBetaVdd_dVbs;
    double dBeta_Vds_0_dVbs;
    double dC1dVbs;
    double dC2dVbs;
    double dBetadVgs;
    double dBetadVds;
    double dBetadVbs;
    double VdsSat = 0;
    double Argl1;
    double Argl2;
    double Vc;
    double Term1;
    double K;
    double Args1;
    double dVcdVgs;
    double dVcdVds;
    double dVcdVbs;
    double dKdVc;
    double dKdVgs;
    double dKdVds;
    double dKdVbs;
    double Args2;
    double Args3;
    double Warg1;
//    double Vcut;
    double N;
    double N0;
    double NB;
    double ND;
    double Warg2;
    double Wds;
    double Wgs;
    double Ilimit;
    double Iexp;
    double Temp1;
    double Vth0;
    double Arg1;
    double Arg2;
    double Arg3;
    double Arg5;
    double Ent;
    double Vcom;
    double Vgb;
    double Vgb_Vfb;
    double VdsPinchoff;
    double EntSquare;
    double Vgs_VthSquare;
    double Argl3;
    double Argl4;
    double Argl5;
    double Argl6;
    double Argl7;
    double Argl8;
    double Argl9;
    double dEntdVds;
    double dEntdVbs;
    double cgbb;
    double cdbb;
    double cbbb;
    double WLCox;
    double Vtsquare;
    double Temp3;
    int ChargeComputationNeeded;
    double co4v15;

    if (ckt->CKTmode & (MODEAC | MODETRAN))
        ChargeComputationNeeded  =  1;
    else if (ckt->CKTmode & MODEINITSMSIG)
        ChargeComputationNeeded  =  1;
    else if ((ckt->CKTmode & MODETRANOP) && (ckt->CKTmode & MODEUIC))
        ChargeComputationNeeded  =  1;
    else
        ChargeComputationNeeded  =  0;

    Vfb  =  inst->B1vfb;
    Phi  =  inst->B1phi;
    K1   =  inst->B1K1;
    K2   =  inst->B1K2;
    Vdd  =  model->B1vdd;
    if((Ugs  =  inst->B1ugs + inst->B1ugsB * vbs) <= 0 ) {
        Ugs = 0;
        dUgsdVbs = 0.0;
    }
    else
        dUgsdVbs = inst->B1ugsB;
    if ((Uds = inst->B1uds + inst->B1udsB * vbs + 
            inst->B1udsD*(vds-Vdd)) <= 0 ) {    
        Uds = 0.0;
        dUdsdVbs = dUdsdVds = 0.0;
    }
    else {
        Leff = inst->B1l * 1.e6 - model->B1deltaL; // Leff in um
        Uds  = Uds / Leff;
        dUdsdVbs = inst->B1udsB / Leff;
        dUdsdVds = inst->B1udsD / Leff;
    }
    Eta = inst->B1eta + inst->B1etaB * vbs + inst->B1etaD * (vds - Vdd);
    if( Eta <= 0 ) {   
        Eta  = 0; 
        dEtadVds = dEtadVbs = 0.0 ;
    }
    else if ( Eta > 1 ) {
        Eta = 1;
        dEtadVds = dEtadVbs = 0;
    }
    else {
        dEtadVds = inst->B1etaD;
        dEtadVbs = inst->B1etaB;
    }
    if( vbs < 0 )
        Vpb = Phi - vbs;
    else
        Vpb = Phi;
    SqrtVpb = sqrt(Vpb);
    Von = Vfb + Phi + K1 * SqrtVpb - K2 * Vpb - Eta * vds;
    Vth = Von;
    dVthdVds = - Eta - dEtadVds * vds;
    dVthdVbs = K2 - 0.5 * K1 / SqrtVpb - dEtadVbs * vds;
    Vgs_Vth  = vgs - Vth;

    G = 1. - 1./(1.744+0.8364 * Vpb);
    A = 1. + 0.5*G*K1/SqrtVpb;
    A = SPMAX( A, 1.0);   // Modified
    Arg  = SPMAX(( 1 + Ugs * Vgs_Vth), 1.0);
    dGdVbs = -0.8364 * (1-G) * (1-G);
    dAdVbs = 0.25 * K1 / SqrtVpb *(2*dGdVbs + G/Vpb);

    if (Vgs_Vth < 0) {
        // cutoff
        DrainCurrent = 0;
        gm = 0;
        gds = 0;
        gmbs = 0;
        goto SubthresholdComputation;
    }

    // Quadratic Interpolation for Beta0 (Beta at vgs  =  0, vds=Vds)

    Beta_Vds_0 = (inst->B1betaZero + inst->B1betaZeroB * vbs);
    BetaVdd = (inst->B1betaVdd + inst->B1betaVddB * vbs);
    dBetaVdd_dVds = SPMAX( inst->B1betaVddD, 0.0); // Modified
    if (vds > Vdd) {
        Beta0 = BetaVdd + dBetaVdd_dVds * (vds - Vdd);
        dBeta0dVds = dBetaVdd_dVds;
        dBeta0dVbs = inst->B1betaVddB;
    }
    else {
        VddSquare = Vdd * Vdd;
        C1 = (-BetaVdd + Beta_Vds_0 + dBetaVdd_dVds * Vdd) / VddSquare;
        C2 = 2 * (BetaVdd - Beta_Vds_0) / Vdd - dBetaVdd_dVds;
        dBeta_Vds_0_dVbs = inst->B1betaZeroB;
        dBetaVdd_dVbs = inst->B1betaVddB;
        dC1dVbs = (dBeta_Vds_0_dVbs - dBetaVdd_dVbs) / VddSquare;
        dC2dVbs = dC1dVbs * (-2) * Vdd;
        Beta0 = (C1 * vds + C2) * vds + Beta_Vds_0;
        dBeta0dVds = 2*C1*vds + C2;
        dBeta0dVbs = dC1dVbs * vds * vds + dC2dVbs * vds + dBeta_Vds_0_dVbs;
    }

    // Beta = Beta0 / ( 1 + Ugs * Vgs_Vth );

    Beta = Beta0 / Arg ;
    dBetadVgs = - Beta * Ugs / Arg;
    dBetadVds = dBeta0dVds / Arg - dBetadVgs * dVthdVds ;
    dBetadVbs = dBeta0dVbs / Arg + Beta * Ugs * dVthdVbs / Arg - 
        Beta * Vgs_Vth * dUgsdVbs / Arg;

    // VdsSat  = SPMAX( Vgs_Vth / ( A + Uds * Vgs_Vth ),  0.0);

    if ((Vc = Uds * Vgs_Vth / A) < 0.0 ) Vc=0.0;
    Term1 = sqrt( 1 + 2 * Vc );
    K = 0.5 * ( 1 + Vc + Term1 );
    VdsSat = SPMAX( Vgs_Vth / ( A * sqrt(K) ) , 0.0 );

    if (vds < VdsSat) {
        // Triode Region
        // Argl1  =  1 + Uds * vds;
        Argl1 = SPMAX( (1 + Uds * vds), 1.);
        Argl2 = Vgs_Vth - 0.5 * A * vds;
        DrainCurrent = Beta * Argl2 * vds / Argl1;
        gm = (dBetadVgs * Argl2 * vds + Beta * vds) / Argl1;
        gds = (dBetadVds * Argl2 * vds + Beta * 
            (Vgs_Vth - vds * dVthdVds - A * vds) - 
            DrainCurrent * (vds * dUdsdVds + Uds )) /  Argl1;
        gmbs = (dBetadVbs * Argl2 * vds + Beta * vds * 
            (- dVthdVbs - 0.5 * vds * dAdVbs ) - 
            DrainCurrent * vds * dUdsdVbs ) / Argl1;
    }
    else {  
        // Pinchoff (Saturation) Region
        Args1 = 1. + 1. / Term1;
        dVcdVgs = Uds / A;
        dVcdVds = Vgs_Vth * dUdsdVds / A - dVcdVgs * dVthdVds;
        dVcdVbs = ( Vgs_Vth * dUdsdVbs - Uds * 
            (dVthdVbs + Vgs_Vth * dAdVbs / A ))/ A;
        dKdVc = 0.5* Args1;
        dKdVgs = dKdVc * dVcdVgs;
        dKdVds = dKdVc * dVcdVds;
        dKdVbs = dKdVc * dVcdVbs;
        Args2 = Vgs_Vth / A / K;
        Args3 = Args2 * Vgs_Vth;
        DrainCurrent = 0.5 * Beta * Args3;
        gm = 0.5 * Args3 * dBetadVgs + Beta * Args2 - 
            DrainCurrent * dKdVgs / K;
        gds = 0.5 * Args3 * dBetadVds - Beta * Args2 * dVthdVds - 
            DrainCurrent * dKdVds / K;
        gmbs = 0.5 * dBetadVbs * Args3 - Beta * Args2 *dVthdVbs - 
            DrainCurrent * (dAdVbs / A + dKdVbs / K );
    }

SubthresholdComputation:

    N0 = inst->B1subthSlope;
//    Vcut = - 40. * N0 * CONSTvt0 ;

// The following 'if' statement has been modified so that subthreshold
// current computation is always executed unless N0 >= 200. This should
// get rid of the Ids kink seen on Ids-Vgs plots at low Vds.
//                                                Peter M. Lee
//                                                6/8/90
//  Old 'if' statement:
//  if( (N0 >=  200) || (Vgs_Vth < Vcut ) || (Vgs_Vth > (-0.5*Vcut)))
  
    if (N0 >=  200)
        goto ChargeComputation;
    
    NB = inst->B1subthSlopeB;
    ND = inst->B1subthSlopeD;
    N  = N0 + NB * vbs + ND * vds; /* subthreshold slope */
    if (N < 0.5) N = 0.5;
    Warg1 = exp( - vds / CONSTvt0 );
    Wds = 1 - Warg1;
    Wgs = exp( Vgs_Vth / ( N * CONSTvt0 ));
    Vtsquare = CONSTvt0 * CONSTvt0 ;
    Warg2 = 6.04965 * Vtsquare * inst->B1betaZero;
    Ilimit = 4.5 * Vtsquare * inst->B1betaZero;
    Iexp = Warg2 * Wgs * Wds;
    DrainCurrent = DrainCurrent + Ilimit * Iexp / ( Ilimit + Iexp );
    Temp1 = Ilimit / ( Ilimit + Iexp );
    Temp1 = Temp1 * Temp1;
    Temp3 = Ilimit / ( Ilimit + Wgs * Warg2 );
    Temp3 = Temp3 * Temp3 * Warg2 * Wgs;
    // if (Temp3 > Ilimit) Temp3=Ilimit;
    gm  =  gm + Temp1 * Iexp / ( N * CONSTvt0 );
    // gds term has been modified to prevent blow up at Vds=0
    gds = gds + Temp3 * ( -Wds / N / CONSTvt0 * (dVthdVds + 
        Vgs_Vth * ND / N ) + Warg1 / CONSTvt0 );
    gmbs = gmbs - Temp1 * Iexp * (dVthdVbs + Vgs_Vth * NB / N ) /  
        (N * CONSTvt0);

ChargeComputation:

    // Some Limiting of DC Parameters
    if (DrainCurrent < 0.0) DrainCurrent = 0.0;
    if (gm < 0.0) gm = 0.0;
    if (gds < 0.0) gds = 0.0;
    if (gmbs < 0.0) gmbs = 0.0;

    WLCox = model->B1Cox * 
        (inst->B1l - model->B1deltaL * 1.e-6) * 
        (inst->B1w - model->B1deltaW * 1.e-6) * 1.e4;   // F

    qg  = 0;
    qd = 0;
    qb = 0;
    cggb = 0;
    cgsb = 0;
    cgdb = 0;
    cdgb = 0;
    cdsb = 0;
    cddb = 0;
    cbgb = 0;
    cbsb = 0;
    cbdb = 0;
    if (!ChargeComputationNeeded)
        goto finished;

    G = 1. - 1./(1.744+0.8364 * Vpb);
    A = 1. + 0.5*G*K1/SqrtVpb;
    A = SPMAX( A, 1.0);   // Modified
    // Arg = 1 + Ugs * Vgs_Vth;
    dGdVbs = -0.8364 * (1-G) * (1-G);
    dAdVbs = 0.25 * K1 / SqrtVpb *(2*dGdVbs + G/Vpb);
    Phi = SPMAX( 0.1, Phi);

    if (model->B1channelChargePartitionFlag >= 1) {

// 0/100 partitioning for drain/source chArges at the saturation region
        Vth0 = Vfb + Phi + K1 * SqrtVpb;
        Vgs_Vth = vgs - Vth0;
        Arg1 = A * vds;
        Arg2 = Vgs_Vth - 0.5 * Arg1;
        Arg3 = vds - Arg1;
        Arg5 = Arg1 * Arg1;
        dVthdVbs =  - 0.5 * K1 / SqrtVpb;
        dAdVbs = 0.5 * K1 * (0.5 * G / Vpb - 0.8364 * (1 -G) * (1 - G) ) / 
            SqrtVpb ;
        Ent = SPMAX(Arg2,1.0e-8);
        dEntdVds =  - 0.5 * A;
        dEntdVbs =  - dVthdVbs - 0.5 * vds * dAdVbs;
        Vcom = Vgs_Vth * Vgs_Vth / 6.0 - 1.25e-1 * Arg1 * 
            Vgs_Vth + 2.5e-2 * Arg5;
        VdsPinchoff = SPMAX( Vgs_Vth / A, 0.0);
        Vgb  =  vgs  -  vbs ;
        Vgb_Vfb  =  Vgb  -  Vfb;

        if (Vgb_Vfb < 0) {
            // Accumulation Region
            qg  =  WLCox * Vgb_Vfb;
            qb  =  - qg;
            cggb  =  WLCox;
            cbgb  =  -WLCox;
            goto finished;
        }
        else if ( vgs < Vth0 ){
            // Subthreshold Region
            qg  =  0.5 * WLCox * K1 * K1 * (-1 + 
                sqrt(1 + 4 * Vgb_Vfb / (K1 * K1)));
            qb  =  -qg;
            cggb  =  WLCox / sqrt(1 + 4 * Vgb_Vfb / (K1 * K1));
            cbgb  =  -cggb;
            goto finished;
        }
        else if( vds < VdsPinchoff) {    // triode region
            // Vgs_Vth2 = Vgs_Vth*Vgs_Vth;
            EntSquare = Ent * Ent;
            Argl1 = 1.2e1 * EntSquare;
            Argl2 = 1.0 - A;
            Argl3 = Arg1 * vds;
            // Argl4 = Vcom/Ent/EntSquare;
            if (Ent > 1.0e-8) {   
                Argl5 = Arg1 / Ent;
                // Argl6 = Vcom/EntSquare;
            }
            else {
                Argl5 = 2.0;
                Argl6 = 4.0 / 1.5e1;
            }
            Argl7 = Argl5 / 1.2e1;
            Argl8 = 6.0 * Ent;
            Argl9 = 0.125 * Argl5 * Argl5;
            qg = WLCox * (vgs - Vfb - Phi - 0.5 * vds + vds * Argl7);
            qb = WLCox * ( - Vth0 + Vfb + Phi + 0.5 * Arg3 - Arg3 * Argl7);
            qd =  - WLCox * (0.5 * Vgs_Vth - 0.75 * Arg1 + 
                0.125 * Arg1 * Argl5);
            cggb = WLCox * (1.0 - Argl3 / Argl1);
            cgdb = WLCox * ( - 0.5 + Arg1 / Argl8 - Argl3 * dEntdVds / 
                Argl1);
            cgbb = WLCox * (vds * vds * dAdVbs * Ent - Argl3 * dEntdVbs) /
                Argl1;
            cgsb =  - (cggb + cgdb + cgbb);
            cbgb = WLCox * Argl3 * Argl2 / Argl1;
            cbdb = WLCox * Argl2 * (0.5 - Arg1 / Argl8 + Argl3 * dEntdVds /
                Argl1);
            cbbb =  - WLCox * (dVthdVbs + 0.5 * vds * dAdVbs + vds * 
                vds * ((1.0 - 2.0 * A) * dAdVbs * Ent - Argl2 * 
                A * dEntdVbs) / Argl1);
            cbsb =  - (cbgb + cbdb + cbbb);
            cdgb =  - WLCox * (0.5 - Argl9);
            cddb = WLCox * (0.75 * A - 0.25 * A * Arg1 / Ent + 
                Argl9 * dEntdVds);
            cdbb = WLCox * (0.5 * dVthdVbs + vds * dAdVbs * 
                (0.75 - 0.25 * Argl5 ) + Argl9 * dEntdVbs);
            cdsb =  - (cdgb + cddb + cdbb);
            goto finished;
        }
        else if( vds >= VdsPinchoff ) {    // saturation region
            Args1 = 1.0 / (3.0 * A);
            qg = WLCox * (vgs - Vfb - Phi - Vgs_Vth * Args1);
            qb = WLCox * (Vfb + Phi - Vth0 + (1.0 - A) * Vgs_Vth * Args1);
            qd = 0.0;
            cggb = WLCox * (1.0 - Args1);
            cgbb = WLCox * Args1 * (dVthdVbs + Vgs_Vth * dAdVbs / A);
            cgsb =  - (cggb + cgdb + cgbb);
            cbgb = WLCox * (Args1 - 1.0 / 3.0);
            cbbb =  - WLCox * ((2.0 / 3.0 + Args1) * dVthdVbs  +  
                Vgs_Vth * Args1 * dAdVbs / A);      // Modified
            cbsb =  - (cbgb + cbdb + cbbb);
            goto finished;
        }

        goto finished;

    }
    else {
        // ChannelChargePartionFlag  < = 0

// 40/60 partitioning for drain/source chArges at the saturation region
        co4v15 = 4./15.;
        Vth0 = Vfb+Phi+K1*SqrtVpb;
        Vgs_Vth = vgs-Vth0;
        Arg1 = A*vds;
        Arg2 = Vgs_Vth-0.5*Arg1;
        Arg3 = vds-Arg1;
        Arg5 = Arg1*Arg1;
        dVthdVbs = -0.5*K1/SqrtVpb;
        dAdVbs = 0.5*K1*(0.5*G/Vpb-0.8364*(1-G)*(1-G))/SqrtVpb;
        Ent = SPMAX(Arg2,1.0e-8);
        dEntdVds = -0.5*A;
        dEntdVbs = -dVthdVbs-0.5*vds*dAdVbs;
        Vcom = Vgs_Vth*Vgs_Vth/6.0-1.25e-1*Arg1*Vgs_Vth+2.5e-2*Arg5;
        VdsPinchoff = SPMAX( Vgs_Vth/A, 0.0);
        Vgb  =  vgs - vbs ;
        Vgb_Vfb  =  Vgb - Vfb;
  
        if (Vgb_Vfb < 0) {           // Accumulation Region
            qg = WLCox * Vgb_Vfb;
            qb = - qg;
            cggb = WLCox;
            cbgb = -WLCox;
            goto finished;
        }
        else if (vgs < Vth0) {    // Subthreshold Region
            qg = 0.5 * WLCox * K1 * K1 * (-1+sqrt(1+4*Vgb_Vfb/(K1*K1)));
            qb = -qg;
            cggb = WLCox/sqrt(1+4*Vgb_Vfb/(K1*K1));
            cbgb = -cggb;
            goto finished;
        }
        else if (vds < VdsPinchoff) {      // triode region
    
            Vgs_VthSquare = Vgs_Vth*Vgs_Vth;
            EntSquare = Ent*Ent;
            Argl1 = 1.2e1*EntSquare;
            Argl2 = 1.0-A;
            Argl3 = Arg1*vds;
            Argl4 = Vcom/Ent/EntSquare;
            if (Ent > 1.0e-8) { 
                Argl5 = Arg1/Ent;
                Argl6 = Vcom/EntSquare;    
            }
            else {
                Argl5 = 2.0;
                Argl6 = 4.0/1.5e1;   
            }
            Argl7 = Argl5/1.2e1;
            Argl8 = 6.0*Ent;
            qg = WLCox*(vgs-Vfb-Phi-0.5*vds+vds*Argl7);
            qb = WLCox*(-Vth0+Vfb+Phi+0.5*Arg3-Arg3*Argl7);
            qd = -WLCox*(0.5*(Vgs_Vth-Arg1)+Arg1*Argl6);
            cggb = WLCox*(1.0-Argl3/Argl1);
            cgdb = WLCox*(-0.5+Arg1/Argl8-Argl3*dEntdVds/Argl1);
            cgbb = WLCox*(vds*vds*dAdVbs*Ent-Argl3*dEntdVbs)/Argl1;
            cgsb = -(cggb+cgdb+cgbb);
            cbgb = WLCox*Argl3*Argl2/Argl1;
            cbdb = WLCox*Argl2*(0.5-Arg1/Argl8+Argl3*dEntdVds/Argl1);
            cbbb = -WLCox*(dVthdVbs+0.5*vds*dAdVbs+vds*vds*((1.0-2.0*A)
                *dAdVbs*Ent-Argl2*A*dEntdVbs)/Argl1);
            cbsb = -(cbgb+cbdb+cbbb);
            cdgb = -WLCox*(0.5+Arg1*(4.0*Vgs_Vth-1.5*Arg1)/Argl1-
                2.0*Arg1*Argl4);
            cddb = WLCox*(0.5*A+2.0*Arg1*dEntdVds*Argl4-A*(2.0*Vgs_VthSquare
                -3.0*Arg1*Vgs_Vth+0.9*Arg5)/Argl1);
            cdbb = WLCox*(0.5*dVthdVbs+0.5*vds*dAdVbs+2.0*Arg1*dEntdVbs
                *Argl4-vds*(2.0*Vgs_VthSquare*dAdVbs-4.0*A*Vgs_Vth*dVthdVbs-3.0
                *Arg1*Vgs_Vth*dAdVbs+1.5*A*Arg1*dVthdVbs+0.9*Arg5*dAdVbs)
                /Argl1);
            cdsb = -(cdgb+cddb+cdbb);
            goto finished;
        }
        else if (vds >= VdsPinchoff) {      // saturation region
     
            Args1 = 1.0/(3.0*A);
            qg = WLCox*(vgs-Vfb-Phi-Vgs_Vth*Args1);
            qb = WLCox*(Vfb+Phi-Vth0+(1.0-A)*Vgs_Vth*Args1);
            qd = -co4v15*WLCox*Vgs_Vth;
            cggb = WLCox*(1.0-Args1);
            cgbb = WLCox*Args1*(dVthdVbs+Vgs_Vth*dAdVbs/A);
            cgsb = -(cggb+cgdb+cgbb);
            cbgb = WLCox*(Args1-1.0/3.0);
            cbbb = -WLCox*((2.0/3.0+Args1)*dVthdVbs+Vgs_Vth*Args1*dAdVbs/A);
            cbsb = -(cbgb+cbdb+cbbb);
            cdgb = -co4v15*WLCox;
            cdbb = co4v15*WLCox*dVthdVbs;
            cdsb = -(cdgb+cddb+cdbb);
            goto finished;
        }
    }

finished:       // returning Values to Calling Routine
       
    *gmPointer = SPMAX(gm,0.0);
    *gdsPointer = SPMAX( gds, 0.0); 
    *gmbsPointer = SPMAX(gmbs,0.0);
    *qgPointer = qg;
    *qbPointer = qb;
    *qdPointer = qd;
    *cggbPointer = cggb;
    *cgdbPointer = cgdb;
    *cgsbPointer = cgsb;
    *cbgbPointer = cbgb;
    *cbdbPointer = cbdb;
    *cbsbPointer = cbsb;
    *cdgbPointer = cdgb;
    *cddbPointer = cddb;
    *cdsbPointer = cdsb; 
    *cdrainPointer = SPMAX(DrainCurrent,0.0);
    *vonPointer = Von;
    *vdsatPointer = VdsSat;
}   
