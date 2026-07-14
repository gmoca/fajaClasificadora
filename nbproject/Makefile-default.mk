#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/pryMicroFaja.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/pryMicroFaja.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=firmware/anti_jam.c firmware/bt_protocol.c firmware/calibration.c firmware/encoder.c firmware/gpio.c firmware/i2c.c firmware/lcd.c firmware/main.c firmware/pwm.c firmware/servo.c firmware/state_machine.c firmware/system.c firmware/tcs34725.c firmware/uart.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/firmware/anti_jam.p1 ${OBJECTDIR}/firmware/bt_protocol.p1 ${OBJECTDIR}/firmware/calibration.p1 ${OBJECTDIR}/firmware/encoder.p1 ${OBJECTDIR}/firmware/gpio.p1 ${OBJECTDIR}/firmware/i2c.p1 ${OBJECTDIR}/firmware/lcd.p1 ${OBJECTDIR}/firmware/main.p1 ${OBJECTDIR}/firmware/pwm.p1 ${OBJECTDIR}/firmware/servo.p1 ${OBJECTDIR}/firmware/state_machine.p1 ${OBJECTDIR}/firmware/system.p1 ${OBJECTDIR}/firmware/tcs34725.p1 ${OBJECTDIR}/firmware/uart.p1
POSSIBLE_DEPFILES=${OBJECTDIR}/firmware/anti_jam.p1.d ${OBJECTDIR}/firmware/bt_protocol.p1.d ${OBJECTDIR}/firmware/calibration.p1.d ${OBJECTDIR}/firmware/encoder.p1.d ${OBJECTDIR}/firmware/gpio.p1.d ${OBJECTDIR}/firmware/i2c.p1.d ${OBJECTDIR}/firmware/lcd.p1.d ${OBJECTDIR}/firmware/main.p1.d ${OBJECTDIR}/firmware/pwm.p1.d ${OBJECTDIR}/firmware/servo.p1.d ${OBJECTDIR}/firmware/state_machine.p1.d ${OBJECTDIR}/firmware/system.p1.d ${OBJECTDIR}/firmware/tcs34725.p1.d ${OBJECTDIR}/firmware/uart.p1.d

# Object Files
OBJECTFILES=${OBJECTDIR}/firmware/anti_jam.p1 ${OBJECTDIR}/firmware/bt_protocol.p1 ${OBJECTDIR}/firmware/calibration.p1 ${OBJECTDIR}/firmware/encoder.p1 ${OBJECTDIR}/firmware/gpio.p1 ${OBJECTDIR}/firmware/i2c.p1 ${OBJECTDIR}/firmware/lcd.p1 ${OBJECTDIR}/firmware/main.p1 ${OBJECTDIR}/firmware/pwm.p1 ${OBJECTDIR}/firmware/servo.p1 ${OBJECTDIR}/firmware/state_machine.p1 ${OBJECTDIR}/firmware/system.p1 ${OBJECTDIR}/firmware/tcs34725.p1 ${OBJECTDIR}/firmware/uart.p1

# Source Files
SOURCEFILES=firmware/anti_jam.c firmware/bt_protocol.c firmware/calibration.c firmware/encoder.c firmware/gpio.c firmware/i2c.c firmware/lcd.c firmware/main.c firmware/pwm.c firmware/servo.c firmware/state_machine.c firmware/system.c firmware/tcs34725.c firmware/uart.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/pryMicroFaja.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=18F4550
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/firmware/anti_jam.p1: firmware/anti_jam.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/anti_jam.p1.d 
	@${RM} ${OBJECTDIR}/firmware/anti_jam.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/anti_jam.p1 firmware/anti_jam.c 
	@-${MV} ${OBJECTDIR}/firmware/anti_jam.d ${OBJECTDIR}/firmware/anti_jam.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/anti_jam.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/bt_protocol.p1: firmware/bt_protocol.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/bt_protocol.p1.d 
	@${RM} ${OBJECTDIR}/firmware/bt_protocol.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/bt_protocol.p1 firmware/bt_protocol.c 
	@-${MV} ${OBJECTDIR}/firmware/bt_protocol.d ${OBJECTDIR}/firmware/bt_protocol.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/bt_protocol.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/calibration.p1: firmware/calibration.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/calibration.p1.d 
	@${RM} ${OBJECTDIR}/firmware/calibration.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/calibration.p1 firmware/calibration.c 
	@-${MV} ${OBJECTDIR}/firmware/calibration.d ${OBJECTDIR}/firmware/calibration.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/calibration.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/encoder.p1: firmware/encoder.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/encoder.p1.d 
	@${RM} ${OBJECTDIR}/firmware/encoder.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/encoder.p1 firmware/encoder.c 
	@-${MV} ${OBJECTDIR}/firmware/encoder.d ${OBJECTDIR}/firmware/encoder.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/encoder.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/gpio.p1: firmware/gpio.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/gpio.p1.d 
	@${RM} ${OBJECTDIR}/firmware/gpio.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/gpio.p1 firmware/gpio.c 
	@-${MV} ${OBJECTDIR}/firmware/gpio.d ${OBJECTDIR}/firmware/gpio.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/gpio.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/i2c.p1: firmware/i2c.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/i2c.p1.d 
	@${RM} ${OBJECTDIR}/firmware/i2c.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/i2c.p1 firmware/i2c.c 
	@-${MV} ${OBJECTDIR}/firmware/i2c.d ${OBJECTDIR}/firmware/i2c.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/i2c.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/lcd.p1: firmware/lcd.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/lcd.p1.d 
	@${RM} ${OBJECTDIR}/firmware/lcd.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/lcd.p1 firmware/lcd.c 
	@-${MV} ${OBJECTDIR}/firmware/lcd.d ${OBJECTDIR}/firmware/lcd.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/lcd.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/main.p1: firmware/main.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/main.p1.d 
	@${RM} ${OBJECTDIR}/firmware/main.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/main.p1 firmware/main.c 
	@-${MV} ${OBJECTDIR}/firmware/main.d ${OBJECTDIR}/firmware/main.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/main.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/pwm.p1: firmware/pwm.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/pwm.p1.d 
	@${RM} ${OBJECTDIR}/firmware/pwm.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/pwm.p1 firmware/pwm.c 
	@-${MV} ${OBJECTDIR}/firmware/pwm.d ${OBJECTDIR}/firmware/pwm.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/pwm.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/servo.p1: firmware/servo.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/servo.p1.d 
	@${RM} ${OBJECTDIR}/firmware/servo.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/servo.p1 firmware/servo.c 
	@-${MV} ${OBJECTDIR}/firmware/servo.d ${OBJECTDIR}/firmware/servo.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/servo.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/state_machine.p1: firmware/state_machine.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/state_machine.p1.d 
	@${RM} ${OBJECTDIR}/firmware/state_machine.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/state_machine.p1 firmware/state_machine.c 
	@-${MV} ${OBJECTDIR}/firmware/state_machine.d ${OBJECTDIR}/firmware/state_machine.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/state_machine.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/system.p1: firmware/system.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/system.p1.d 
	@${RM} ${OBJECTDIR}/firmware/system.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/system.p1 firmware/system.c 
	@-${MV} ${OBJECTDIR}/firmware/system.d ${OBJECTDIR}/firmware/system.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/system.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/tcs34725.p1: firmware/tcs34725.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/tcs34725.p1.d 
	@${RM} ${OBJECTDIR}/firmware/tcs34725.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/tcs34725.p1 firmware/tcs34725.c 
	@-${MV} ${OBJECTDIR}/firmware/tcs34725.d ${OBJECTDIR}/firmware/tcs34725.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/tcs34725.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/uart.p1: firmware/uart.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/uart.p1.d 
	@${RM} ${OBJECTDIR}/firmware/uart.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1  -mdebugger=none   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/uart.p1 firmware/uart.c 
	@-${MV} ${OBJECTDIR}/firmware/uart.d ${OBJECTDIR}/firmware/uart.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/uart.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
else
${OBJECTDIR}/firmware/anti_jam.p1: firmware/anti_jam.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/anti_jam.p1.d 
	@${RM} ${OBJECTDIR}/firmware/anti_jam.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/anti_jam.p1 firmware/anti_jam.c 
	@-${MV} ${OBJECTDIR}/firmware/anti_jam.d ${OBJECTDIR}/firmware/anti_jam.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/anti_jam.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/bt_protocol.p1: firmware/bt_protocol.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/bt_protocol.p1.d 
	@${RM} ${OBJECTDIR}/firmware/bt_protocol.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/bt_protocol.p1 firmware/bt_protocol.c 
	@-${MV} ${OBJECTDIR}/firmware/bt_protocol.d ${OBJECTDIR}/firmware/bt_protocol.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/bt_protocol.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/calibration.p1: firmware/calibration.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/calibration.p1.d 
	@${RM} ${OBJECTDIR}/firmware/calibration.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/calibration.p1 firmware/calibration.c 
	@-${MV} ${OBJECTDIR}/firmware/calibration.d ${OBJECTDIR}/firmware/calibration.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/calibration.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/encoder.p1: firmware/encoder.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/encoder.p1.d 
	@${RM} ${OBJECTDIR}/firmware/encoder.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/encoder.p1 firmware/encoder.c 
	@-${MV} ${OBJECTDIR}/firmware/encoder.d ${OBJECTDIR}/firmware/encoder.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/encoder.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/gpio.p1: firmware/gpio.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/gpio.p1.d 
	@${RM} ${OBJECTDIR}/firmware/gpio.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/gpio.p1 firmware/gpio.c 
	@-${MV} ${OBJECTDIR}/firmware/gpio.d ${OBJECTDIR}/firmware/gpio.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/gpio.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/i2c.p1: firmware/i2c.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/i2c.p1.d 
	@${RM} ${OBJECTDIR}/firmware/i2c.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/i2c.p1 firmware/i2c.c 
	@-${MV} ${OBJECTDIR}/firmware/i2c.d ${OBJECTDIR}/firmware/i2c.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/i2c.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/lcd.p1: firmware/lcd.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/lcd.p1.d 
	@${RM} ${OBJECTDIR}/firmware/lcd.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/lcd.p1 firmware/lcd.c 
	@-${MV} ${OBJECTDIR}/firmware/lcd.d ${OBJECTDIR}/firmware/lcd.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/lcd.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/main.p1: firmware/main.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/main.p1.d 
	@${RM} ${OBJECTDIR}/firmware/main.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/main.p1 firmware/main.c 
	@-${MV} ${OBJECTDIR}/firmware/main.d ${OBJECTDIR}/firmware/main.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/main.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/pwm.p1: firmware/pwm.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/pwm.p1.d 
	@${RM} ${OBJECTDIR}/firmware/pwm.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/pwm.p1 firmware/pwm.c 
	@-${MV} ${OBJECTDIR}/firmware/pwm.d ${OBJECTDIR}/firmware/pwm.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/pwm.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/servo.p1: firmware/servo.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/servo.p1.d 
	@${RM} ${OBJECTDIR}/firmware/servo.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/servo.p1 firmware/servo.c 
	@-${MV} ${OBJECTDIR}/firmware/servo.d ${OBJECTDIR}/firmware/servo.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/servo.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/state_machine.p1: firmware/state_machine.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/state_machine.p1.d 
	@${RM} ${OBJECTDIR}/firmware/state_machine.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/state_machine.p1 firmware/state_machine.c 
	@-${MV} ${OBJECTDIR}/firmware/state_machine.d ${OBJECTDIR}/firmware/state_machine.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/state_machine.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/system.p1: firmware/system.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/system.p1.d 
	@${RM} ${OBJECTDIR}/firmware/system.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/system.p1 firmware/system.c 
	@-${MV} ${OBJECTDIR}/firmware/system.d ${OBJECTDIR}/firmware/system.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/system.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/tcs34725.p1: firmware/tcs34725.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/tcs34725.p1.d 
	@${RM} ${OBJECTDIR}/firmware/tcs34725.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/tcs34725.p1 firmware/tcs34725.c 
	@-${MV} ${OBJECTDIR}/firmware/tcs34725.d ${OBJECTDIR}/firmware/tcs34725.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/tcs34725.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/firmware/uart.p1: firmware/uart.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/firmware" 
	@${RM} ${OBJECTDIR}/firmware/uart.p1.d 
	@${RM} ${OBJECTDIR}/firmware/uart.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits $(COMPARISON_BUILD)  -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     -o ${OBJECTDIR}/firmware/uart.p1 firmware/uart.c 
	@-${MV} ${OBJECTDIR}/firmware/uart.d ${OBJECTDIR}/firmware/uart.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/firmware/uart.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/pryMicroFaja.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -Wl,-Map=${DISTDIR}/pryMicroFaja.X.${IMAGE_TYPE}.map  -D__DEBUG=1  -mdebugger=none  -DXPRJ_default=$(CND_CONF)  -Wl,--defsym=__MPLAB_BUILD=1   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto        $(COMPARISON_BUILD) -Wl,--memorysummary,${DISTDIR}/memoryfile.xml -o ${DISTDIR}/pryMicroFaja.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}     
	@${RM} ${DISTDIR}/pryMicroFaja.X.${IMAGE_TYPE}.hex 
	
	
else
${DISTDIR}/pryMicroFaja.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -Wl,-Map=${DISTDIR}/pryMicroFaja.X.${IMAGE_TYPE}.map  -DXPRJ_default=$(CND_CONF)  -Wl,--defsym=__MPLAB_BUILD=1   -mdfp="${DFP_DIR}/xc8"  -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -mwarn=-3 -Wa,-a -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mno-keep-startup -mno-download -mno-default-config-bits -std=c99 -gdwarf-3 -mstack=compiled:auto:auto:auto     $(COMPARISON_BUILD) -Wl,--memorysummary,${DISTDIR}/memoryfile.xml -o ${DISTDIR}/pryMicroFaja.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}     
	
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(wildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
