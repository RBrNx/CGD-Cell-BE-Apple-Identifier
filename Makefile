## --------------------------------------------------------------  
# (C)Copyright 2001,2007,                                         
# International Business Machines Corporation,                    
# Sony Computer Entertainment, Incorporated,                      
# Toshiba Corporation,                                            
#                                                                 
# All Rights Reserved.                                            
#                                                                 
# Redistribution and use in source and binary forms, with or      
# without modification, are permitted provided that the           
# following conditions are met:                                   
#                                                                 
# - Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer. 
#                                                                 
# - Redistributions in binary form must reproduce the above       
#   copyright notice, this list of conditions and the following   
#   disclaimer in the documentation and/or other materials        
#   provided with the distribution.                               
#                                                                 
# - Neither the name of IBM Corporation nor the names of its      
#   contributors may be used to endorse or promote products       
#   derived from this software without specific prior written     
#   permission.                                                   
#                                                                 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND          
# CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,     
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE        
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR            
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT    
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;    
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)        
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN       
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR    
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              
# --------------------------------------------------------------  
# PROLOG END TAG zYx                                              

########################################################################
#			Subdirectories
########################################################################

DIRS := SPU-Greyscale SPU-Gaussian SPU-Sobel SPU-DoubleT SPU-Hyst

########################################################################
#                       Target
########################################################################

PROGRAM_ppu	:= ParaAppleIdentifier_prog


########################################################################
#			Imports
########################################################################

IMPORTS := SPU-Greyscale/lib_greyscale_spu.a SPU-Gaussian/lib_gaussian_spu.a SPU-Sobel/lib_sobel_spu.a SPU-DoubleT/lib_doublet_spu.a SPU-Hyst/lib_hyst_spu.a -lspe2 -lpthread


########################################################################
#                       Local Defines
########################################################################

INSTALL_DIR	= ~/ParaAppleIdentifier
INSTALL_FILES	= $(PROGRAM_ppu)



########################################################################
#			buildutils/make.footer
########################################################################

ifdef CELL_TOP
	include $(CELL_TOP)/buildutils/make.footer
else
	include ../../../buildutils/make.footer
endif
