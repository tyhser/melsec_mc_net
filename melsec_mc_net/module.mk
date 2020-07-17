#Sources
MC_LIB_SRC = melsec_mc_net
MC_LIB_FILES = $(MC_LIB_SRC)/melsec_helper.c \
			   $(MC_LIB_SRC)/melsec_mc_ascii.c \
			   $(MC_LIB_SRC)/melsec_mc_bin.c  \
			   $(MC_LIB_SRC)/melsec_mc_comm.c \
			   $(MC_LIB_SRC)/socket.c \
			   $(MC_LIB_SRC)/utill.c

ifeq ($(FEATURE_XX), y)
	CFLAGS += -DFEATURE_XX
endif
CFILES += $(MC_LIB_FILES)

# include path
CFLAGS += -I$(SOURCE_DIR)/melsec_mc_net
