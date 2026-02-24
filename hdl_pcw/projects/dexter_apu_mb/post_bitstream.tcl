set old_dir [pwd]
cd [file dirname [file normalize [info script]]]

write_hw_platform -fixed -force -file dexter_apu_mb.xsa

cd $old_dir
