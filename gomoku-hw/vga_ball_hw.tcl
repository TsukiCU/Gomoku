# TCL File Generated by Component Editor 21.1
# Wed Apr 24 19:30:31 EDT 2024
# DO NOT MODIFY


# 
# vga_ball "VGA BALL" v1.0
#  2024.04.24.19:30:31
# 
# 

# 
# request TCL package from ACDS 16.1
# 
package require -exact qsys 16.1


# 
# module vga_ball
# 
set_module_property DESCRIPTION ""
set_module_property NAME vga_ball
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR ""
set_module_property DISPLAY_NAME "VGA BALL"
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false
set_module_property REPORT_HIERARCHY false


# 
# file sets
# 
add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL vga_ball
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file vga_ball.sv SYSTEM_VERILOG PATH vga_ball.sv TOP_LEVEL_FILE


# 
# parameters
# 


# 
# module assignments
# 
set_module_assignment embeddedsw.dts.group vga
set_module_assignment embeddedsw.dts.name vga_ball
set_module_assignment embeddedsw.dts.vendor csee4840


# 
# display items
# 


# 
# connection point clock
# 
add_interface clock clock end
set_interface_property clock clockRate 0
set_interface_property clock ENABLED true
set_interface_property clock EXPORT_OF ""
set_interface_property clock PORT_NAME_MAP ""
set_interface_property clock CMSIS_SVD_VARIABLES ""
set_interface_property clock SVD_ADDRESS_GROUP ""

add_interface_port clock clk clk Input 1


# 
# connection point reset
# 
add_interface reset reset end
set_interface_property reset associatedClock clock
set_interface_property reset synchronousEdges DEASSERT
set_interface_property reset ENABLED true
set_interface_property reset EXPORT_OF ""
set_interface_property reset PORT_NAME_MAP ""
set_interface_property reset CMSIS_SVD_VARIABLES ""
set_interface_property reset SVD_ADDRESS_GROUP ""

add_interface_port reset reset reset Input 1


# 
# connection point avalon_slave_0
# 
add_interface avalon_slave_0 avalon end
set_interface_property avalon_slave_0 addressUnits WORDS
set_interface_property avalon_slave_0 associatedClock clock
set_interface_property avalon_slave_0 associatedReset reset
set_interface_property avalon_slave_0 bitsPerSymbol 8
set_interface_property avalon_slave_0 burstOnBurstBoundariesOnly false
set_interface_property avalon_slave_0 burstcountUnits WORDS
set_interface_property avalon_slave_0 explicitAddressSpan 0
set_interface_property avalon_slave_0 holdTime 0
set_interface_property avalon_slave_0 linewrapBursts false
set_interface_property avalon_slave_0 maximumPendingReadTransactions 0
set_interface_property avalon_slave_0 maximumPendingWriteTransactions 0
set_interface_property avalon_slave_0 readLatency 0
set_interface_property avalon_slave_0 readWaitTime 1
set_interface_property avalon_slave_0 setupTime 0
set_interface_property avalon_slave_0 timingUnits Cycles
set_interface_property avalon_slave_0 writeWaitTime 0
set_interface_property avalon_slave_0 ENABLED true
set_interface_property avalon_slave_0 EXPORT_OF ""
set_interface_property avalon_slave_0 PORT_NAME_MAP ""
set_interface_property avalon_slave_0 CMSIS_SVD_VARIABLES ""
set_interface_property avalon_slave_0 SVD_ADDRESS_GROUP ""

add_interface_port avalon_slave_0 writedata writedata Input 16
add_interface_port avalon_slave_0 write write Input 1
add_interface_port avalon_slave_0 chipselect chipselect Input 1
add_interface_port avalon_slave_0 address address Input 3
set_interface_assignment avalon_slave_0 embeddedsw.configuration.isFlash 0
set_interface_assignment avalon_slave_0 embeddedsw.configuration.isMemoryDevice 0
set_interface_assignment avalon_slave_0 embeddedsw.configuration.isNonVolatileStorage 0
set_interface_assignment avalon_slave_0 embeddedsw.configuration.isPrintableDevice 0


# 
# connection point vga
# 
add_interface vga conduit end
set_interface_property vga associatedClock clock
set_interface_property vga associatedReset ""
set_interface_property vga ENABLED true
set_interface_property vga EXPORT_OF ""
set_interface_property vga PORT_NAME_MAP ""
set_interface_property vga CMSIS_SVD_VARIABLES ""
set_interface_property vga SVD_ADDRESS_GROUP ""

add_interface_port vga VGA_B b Output 8
add_interface_port vga VGA_BLANK_n blank_n Output 1
add_interface_port vga VGA_CLK clk Output 1
add_interface_port vga VGA_G g Output 8
add_interface_port vga VGA_HS hs Output 1
add_interface_port vga VGA_R r Output 8
add_interface_port vga VGA_SYNC_n sync_n Output 1
add_interface_port vga VGA_VS vs Output 1


# 
# connection point avalon_streaming_source_l
# 
add_interface avalon_streaming_source_l avalon_streaming start
set_interface_property avalon_streaming_source_l associatedClock clock
set_interface_property avalon_streaming_source_l associatedReset reset
set_interface_property avalon_streaming_source_l dataBitsPerSymbol 8
set_interface_property avalon_streaming_source_l errorDescriptor ""
set_interface_property avalon_streaming_source_l firstSymbolInHighOrderBits true
set_interface_property avalon_streaming_source_l maxChannel 0
set_interface_property avalon_streaming_source_l readyLatency 0
set_interface_property avalon_streaming_source_l ENABLED true
set_interface_property avalon_streaming_source_l EXPORT_OF ""
set_interface_property avalon_streaming_source_l PORT_NAME_MAP ""
set_interface_property avalon_streaming_source_l CMSIS_SVD_VARIABLES ""
set_interface_property avalon_streaming_source_l SVD_ADDRESS_GROUP ""

add_interface_port avalon_streaming_source_l L_DATA data Output 16
add_interface_port avalon_streaming_source_l L_READY ready Input 1
add_interface_port avalon_streaming_source_l L_VALID valid Output 1


# 
# connection point avalon_streaming_source_r
# 
add_interface avalon_streaming_source_r avalon_streaming start
set_interface_property avalon_streaming_source_r associatedClock clock
set_interface_property avalon_streaming_source_r associatedReset reset
set_interface_property avalon_streaming_source_r dataBitsPerSymbol 8
set_interface_property avalon_streaming_source_r errorDescriptor ""
set_interface_property avalon_streaming_source_r firstSymbolInHighOrderBits true
set_interface_property avalon_streaming_source_r maxChannel 0
set_interface_property avalon_streaming_source_r readyLatency 0
set_interface_property avalon_streaming_source_r ENABLED true
set_interface_property avalon_streaming_source_r EXPORT_OF ""
set_interface_property avalon_streaming_source_r PORT_NAME_MAP ""
set_interface_property avalon_streaming_source_r CMSIS_SVD_VARIABLES ""
set_interface_property avalon_streaming_source_r SVD_ADDRESS_GROUP ""

add_interface_port avalon_streaming_source_r R_DATA data Output 16
add_interface_port avalon_streaming_source_r R_READY ready Input 1
add_interface_port avalon_streaming_source_r R_VALID valid Output 1

