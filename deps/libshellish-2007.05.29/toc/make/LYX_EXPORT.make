#!/do/not/make
#
# A toc Makefile snippet to export input files using LyX (http://www.lyx.org).
#
# Usage:
#
# LYX_EXPORT = mylyxfile myotherfile ... (WITHOUT file extensions)
# LYX_EXPORT_FORMATS = format list, as expected by 'lyx -e XXX'. Defaults to 'html pdf'.
# LYX_EXPORT_ARGS = optional args to pass to lyx
# # optional: mylyxfile_lyx_FORMATS = defaults to LYX_EXPORT_FORMATS
# # optional: mylyxfile_lyx_EXPORT_ARGS = defaults to LYX_EXPORT_ARGS
# include $(TOC_MAKESDIR)/LYX_EXPORT.make
#
# all: LYX_EXPORT
#
# Requires:
#     LYX_BIN = /path/to/lyx
#     Any exporters which lyx may need.



LYX_EXPORT_MAKE = $(TOC_MAKESDIR)/LYX_EXPORT.make
LYX_EXPORT_RULES_GENERATOR = $(TOC_MAKESDIR)/makerules.LYX_EXPORT
LYX_EXPORT_INCFILE = .toc.LYX_EXPORT.make
CLEAN_FILES += $(LYX_EXPORT_INCFILE)
$(LYX_EXPORT_INCFILE): $(LYX_EXPORT_RULES_GENERATOR) $(LYX_EXPORT_MAKE) Makefile
ifeq (1,$(MAKING_CLEAN))
        @echo "$(MAKECMDGOALS): skipping LYX_EXPORT rules generation."
else
	@echo "Generating LYX_EXPORT rules."; \
	$(call toc_generate_rules,LYX_EXPORT,$(LYX_EXPORT)) > $@
endif
-include $(LYX_EXPORT_INCFILE)

.PHONY: LYX_EXPORT
LYX_EXPORT:
lyx: LYX_EXPORT


