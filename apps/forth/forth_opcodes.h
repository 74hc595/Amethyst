/**
 * Reference material on Forth word frequency:
 *
 * "The Common Case in Forth Programs" by Gregg/Ertl/Waldron:
 *   http://www.complang.tuwien.ac.at/anton/euroforth/ef01/gregg01.pdf
 *
 * "Stack Computers: The New Wave" by Koopman:
 *   https://users.ece.cmu.edu/~koopman/stack_computers/stack_computers_book.pdf
 */
#ifndef FORTH_OPCODES_H_
#define FORTH_OPCODES_H_

#include "forth_opcode_types.h"

#define OPCODES \
  X(0,   "BREAK",       1, break,           break,           OP_COMPILEONLY) \
  X(1,   "(CALL)",     2C, call,            call,            0) \
  X(2,   "",           2C, call,            call_02,         0) \
  X(3,   "",           2C, call,            call_03,         0) \
  X(4,   "",           2C, call,            call_04,         0) \
  X(5,   "",           2C, call,            call_05,         0) \
  X(6,   "",           2C, call,            call_06,         0) \
  X(7,   "",           2C, call,            call_07,         0) \
  X(8,   "",           2C, call,            call_08,         0) \
  X(9,   "",           2C, call,            call_09,         0) \
  X(10,  "",           2C, call,            call_0a,         0) \
  X(11,  "",           2C, call,            call_0b,         0) \
  X(12,  "",           2C, call,            call_0c,         0) \
  X(13,  "",           2C, call,            call_0d,         0) \
  X(14,  "",           2C, call,            call_0e,         0) \
  X(15,  "",           2C, call,            call_0f,         0) \
  X(16,  "",           2C, call,            call_10,         0) \
  X(17,  "",           2C, call,            call_11,         0) \
  X(18,  "",           2C, call,            call_12,         0) \
  X(19,  "",           2C, call,            call_13,         0) \
  X(20,  "",           2C, call,            call_14,         0) \
  X(21,  "",           2C, call,            call_15,         0) \
  X(22,  "",           2C, call,            call_16,         0) \
  X(23,  "",           2C, call,            call_17,         0) \
  X(24,  "",           2C, call,            call_18,         0) \
  X(25,  "",           2C, call,            call_19,         0) \
  X(26,  "",           2C, call,            call_1a,         0) \
  X(27,  "",           2C, call,            call_1b,         0) \
  X(28,  "",           2C, call,            call_1c,         0) \
  X(29,  "",           2C, call,            call_1d,         0) \
  X(30,  "",           2C, call,            call_1e,         0) \
  X(31,  "",           2C, call,            call_1f,         0) \
  X(32,  "",           2C, call,            call_20,         0) \
  X(33,  "",           2C, call,            call_21,         0) \
  X(34,  "",           2C, call,            call_22,         0) \
  X(35,  "",           2C, call,            call_23,         0) \
  X(36,  "",           2C, call,            call_24,         0) \
  X(37,  "",           2C, call,            call_25,         0) \
  X(38,  "",           2C, call,            call_26,         0) \
  X(39,  "",           2C, call,            call_27,         0) \
  X(40,  "",           2C, call,            call_28,         0) \
  X(41,  "",           2C, call,            call_29,         0) \
  X(42,  "",           2C, call,            call_2a,         0) \
  X(43,  "",           2C, call,            call_2b,         0) \
  X(44,  "",           2C, call,            call_2c,         0) \
  X(45,  "",           2C, call,            call_2d,         0) \
  X(46,  "",           2C, call,            call_2e,         0) \
  X(47,  "",           2C, call,            call_2f,         0) \
  X(48,  "",           2C, call,            call_30,         0) \
  X(49,  "",           2C, call,            call_31,         0) \
  X(50,  "",           2C, call,            call_32,         0) \
  X(51,  "",           2C, call,            call_33,         0) \
  X(52,  "",           2C, call,            call_34,         0) \
  X(53,  "",           2C, call,            call_35,         0) \
  X(54,  "",           2C, call,            call_36,         0) \
  X(55,  "",           2C, call,            call_37,         0) \
  X(56,  "",           2C, call,            call_38,         0) \
  X(57,  "",           2C, call,            call_39,         0) \
  X(58,  "",           2C, call,            call_3a,         0) \
  X(59,  "",           2C, call,            call_3b,         0) \
  X(60,  "",           2C, call,            call_3c,         0) \
  X(61,  "",           2C, call,            call_3d,         0) \
  X(62,  "",           2C, call,            call_3e,         0) \
  X(63,  "",           2C, call,            call_3f,         0) \
  X(64,  "",           2C, call,            call_40,         0) \
  X(65,  "0",           1, zero,            zero,            OP_VISIBLE) \
  X(66,  "-1",          1, minus1,          minus1,          OP_VISIBLE) \
  X(67,  "(C#)",       2U, lit8,            lit8,            OP_COMPILEONLY) \
  X(68,  "(#)",        3S, lit16,           lit16,           OP_COMPILEONLY) \
  X(69,  "(S\")",     STR, litstr,          litstr,          OP_COMPILEONLY) \
  X(70,  "DUP",         1, dup,             dup,             OP_VISIBLE) \
  X(71,  "DROP",        1, drop,            drop,            OP_VISIBLE) \
  X(72,  "NIP",         1, nip,             nip,             OP_VISIBLE) \
  X(73,  "SWAP",        1, swap,            swap,            OP_VISIBLE) \
  X(74,  "OVER",        1, over,            over,            OP_VISIBLE) \
  X(75,  "ROT",         1, rot,             rot,             OP_VISIBLE) \
  X(76,  "2DUP",        1, twodup,          twodup,          OP_VISIBLE) \
  X(77,  "2DROP",       1, twodrop,         twodrop,         OP_VISIBLE) \
  X(78,  "R@",          1, rfetch,          rfetch,          OP_VISIBLE) \
  X(79,  "(C#+R@)",    2U, rfetchlitoffset, rfetchlitoffset, OP_COMPILEONLY|OP_USES_RSTACK) \
  X(80,  ">R",          1, tor,             tor,             OP_VISIBLE|OP_USES_RSTACK) \
  X(81,  "R>",          1, rfrom,           rfrom,           OP_VISIBLE|OP_USES_RSTACK) \
  X(82,  "RDROP",       1, rdrop,           rdrop,           OP_VISIBLE|OP_USES_RSTACK) \
  X(83,  "2RDROP",      1, twordrop,        twordrop,        OP_VISIBLE|OP_USES_RSTACK) \
  X(84,  "2R@",         1, tworfetch,       tworfetch,       OP_VISIBLE|OP_USES_RSTACK) \
  X(85,  "2>R",         1, twotor,          twotor,          OP_VISIBLE|OP_USES_RSTACK) \
  X(86,  "2R>",         1, tworfrom,        tworfrom,        OP_VISIBLE|OP_USES_RSTACK) \
  X(87,  "(R>LINK)",    1, link,            link,            0) \
  X(88,  "(LINK>IP)",   1, unlink,          unlink,          0) \
  X(89,  "0=",          1, eq0,             eq0,             OP_VISIBLE) \
  X(90,  "0<",          1, lt0,             lt0,             OP_VISIBLE) \
  X(91,  "0>",          1, gt0,             gt0,             OP_VISIBLE) \
  X(92,  "=",           1, eq,              eq,              OP_VISIBLE) \
  X(93,  "<",           1, lt,              lt,              OP_VISIBLE) \
  X(94,  ">",           1, gt,              gt,              OP_VISIBLE) \
  X(95,  "U<",          1, ult,             ult,             OP_VISIBLE) \
  X(96,  "U>",          1, ugt,             ugt,             OP_VISIBLE) \
  X(97,  "1+",          1, oneplus,         oneplus,         OP_VISIBLE) \
  X(98,  "1-",          1, oneminus,        oneminus,        OP_VISIBLE) \
  X(99,  "2+",          1, twoplus,         twoplus,         OP_VISIBLE) \
  X(100, "2*",          1, twostar,         twostar,         OP_VISIBLE) \
  X(101, "2/",          1, twoslash,        twoslash,        OP_VISIBLE) \
  X(102, "U2/",         1, utwoslash,       utwoslash,       OP_VISIBLE) \
  X(103, "ABS",         1, abs,             abs,             OP_VISIBLE) \
  X(104, "NEGATE",      1, negate,          negate,          OP_VISIBLE) \
  X(105, "+",           1, plus,            plus,            OP_VISIBLE) \
  X(106, "-",           1, minus,           minus,           OP_VISIBLE) \
  X(107, "INVERT",      1, invert,          invert,          OP_VISIBLE) \
  X(108, "AND",         1, and,             and,             OP_VISIBLE) \
  X(109, "(C#AND)",    2U, candlit,         candlit,         OP_COMPILEONLY) \
  X(110, "OR",          1, or,              or,              OP_VISIBLE) \
  X(111, "XOR",         1, xor,             xor,             OP_VISIBLE) \
  X(112, "C@",          1, cfetch,          cfetch,          OP_VISIBLE) \
  X(113, "C!",          1, cstore,          cstore,          OP_VISIBLE) \
  X(114, "(#C@)",      3X, cfetchlit,       cfetchlit,       OP_COMPILEONLY) \
  X(115, "(#C!)",      3X, cstorelit,       cstorelit,       OP_COMPILEONLY) \
  X(116, "(C#+HC@)",   2U, cfetchlithi,     cfetchlithi,     OP_COMPILEONLY) \
  X(117, "(C#+HC!)",   2U, cstorelithi,     cstorelithi,     OP_COMPILEONLY) \
  X(118, "C@+",         1, cfetchplus,      cfetchplus,      OP_VISIBLE) \
  X(119, "C!+",         1, cstoreplus,      cstoreplus,      OP_VISIBLE) \
  X(120, "(C#+HCBITS@)",3X,cfetchbitshi,    cfetchbitshi,    OP_COMPILEONLY) \
  X(121, "(C#+HCBITS!)",3X,cstorebitshi,    cstorebitshi,    OP_COMPILEONLY) \
  X(122, "@",           1, fetch,           fetch,           OP_VISIBLE) \
  X(123, "!",           1, store,           store,           OP_VISIBLE) \
  X(124, "+!",          1, plusstore,       plusstore,       OP_VISIBLE) \
  X(125, "(#@)",       3X, fetchlit,        fetchlit,        OP_COMPILEONLY) \
  X(126, "(#!)",       3X, storelit,        storelit,        OP_COMPILEONLY) \
  X(127, "(C#+H@)",    2U, fetchlithi,      fetchlithi,      OP_COMPILEONLY) \
  X(128, "(C#+H!)",    2U, storelithi,      storelithi,      OP_COMPILEONLY) \
  X(129, "2@",          1, twofetch,        twofetch,        OP_VISIBLE) \
  X(130, "2!",          1, twostore,        twostore,        OP_VISIBLE) \
  X(131, "SELECT",      1, select,          select,          OP_VISIBLE) \
  X(132, "(?DO)",      2F, qdo,             qdo,             OP_COMPILEONLY) \
  X(133, "(LOOP)",     2B, loop,            loop,            OP_COMPILEONLY) \
  X(134, "(+LOOP)",    2B, plusloop,        plusloop,        OP_COMPILEONLY) \
  X(135, "(XY-LOOP)",   1, xyloop,          xyloop,          OP_COMPILEONLY) \
  X(136, "(OF)",       2F, of,              of,              OP_COMPILEONLY) \
  X(137, "(BRANCH<)",  2B, branchback,      branchback,      OP_COMPILEONLY) \
  X(138, "(BRANCH>)",  2F, branchfwd,       branchfwd,       OP_COMPILEONLY) \
  X(139, "(0BRANCH<)", 2B, zbranchback,     zbranchback,     OP_COMPILEONLY) \
  X(140, "(0BRANCH>)", 2F, zbranchfwd,      zbranchfwd,      OP_COMPILEONLY) \
  X(141, "(NP0+CALL)", 3X, np0relcall,      np0relcall,      OP_COMPILEONLY) \
  X(142, "(CCALL)",    3X, ccall,           ccall,           OP_COMPILEONLY) \
  X(143, "(JUMP)",     3X, jump,            jump,            OP_COMPILEONLY) \
  X(144, "(NP0+JUMP)", 3X, np0reljump,      np0reljump,      OP_COMPILEONLY) \
  X(145, "(DODOES)",   3X, dodoes,          dodoes,          OP_COMPILEONLY) \
  X(146, "THROW",       1, throw,           throw,           OP_VISIBLE) \
  X(147, "EMIT",        1, emit,            emit,            OP_VISIBLE) \
  X(148, "(C#EMIT)",   2U, emitlit,         emitlit,         OP_COMPILEONLY) \
  X(149, "H.",          1, hdot,            hdot,            OP_VISIBLE) \
  X(150, "HOLD",        1, hold,            hold,            OP_VISIBLE) \
  X(151, "#",           1, num,             num,             OP_VISIBLE) \
  X(152, "TYPE",        1, type,            type,            OP_VISIBLE) \
  X(153, "KEY",         1, key,             key,             OP_VISIBLE) \
  X(154, "ALLOT",       1, allot,           allot,           OP_VISIBLE) \
  X(155, "HERE",        1, here,            here,            OP_VISIBLE) \
  X(156, "C,",          1, ccomma,          ccomma,          OP_VISIBLE) \
  X(157, ",",           1, comma,           comma,           OP_VISIBLE) \
  X(158, "X,",          1, xcomma,          xcomma,          OP_VISIBLE) \
  X(159, "XALLOT",      1, xallot,          xallot,          OP_VISIBLE) /**/ \
  X(160, "(C#,)",      2U, litccomma,       litccomma,       OP_COMPILEONLY) \
  X(161, "(HEADER)",   2X, header,          header,          OP_COMPILEONLY) \
  X(162, "(C::DOES>)", 3X, makechild,       makechild,       OP_COMPILEONLY) \
  X(163, "(DOES>)",    3X, does,            does,            OP_COMPILEONLY) \
  X(164, "PARSE-NAME",  1, parsename,       parsename,       OP_VISIBLE) \
  X(165, "PARSE",       1, parse,           parse,           OP_VISIBLE) \
  X(166, "'",           1, tick,            tick,            OP_VISIBLE) \
  X(167, "EXECUTE",     1, execute,         execute,         OP_VISIBLE) \
  X(168, "COMPILE,",    1, compile,         compile,         OP_VISIBLE) \
  X(169, "(MARK)",     2X, mark,            mark,            OP_COMPILEONLY) \
  X(170, "FILL",        1, fill,            fill,            OP_VISIBLE) /**/ \
  X(171, "PUTC",        1, putc,            putc,            OP_VISIBLE) \
  X(172, "><",          1, byteswap,        byteswap,        OP_VISIBLE) \
  X(173, "LH>",         1, splitbytes,      splitbytes,      OP_VISIBLE) \
  X(174, ">LH",         1, mergebytes,      mergebytes,      OP_VISIBLE) \
  X(175, ">H",          1, tohighbyte,      tohighbyte,      OP_VISIBLE) \
  X(176, ">L",          1, tolowbyte,       tolowbyte,       OP_VISIBLE) \
  X(177, "LO",          1, lowbyte,         lowbyte,         OP_VISIBLE) \
  X(178, "HI",          1, highbyte,        highbyte,        OP_VISIBLE) \
  X(179, "LH+",         1, bytepairplus,    bytepairplus,    OP_VISIBLE) \
  X(180, "LH-",         1, bytepairminus,   bytepairminus,   OP_VISIBLE) \
  X(181, "L+",          1, lplus,           lplus,           OP_VISIBLE) \
  X(182, "H+",          1, hplus,           hplus,           OP_VISIBLE) \
  X(183, "LNEGATE",     1, lnegate,         lnegate,         OP_VISIBLE) \
  X(184, "HNEGATE",     1, hnegate,         hnegate,         OP_VISIBLE) \
  X(185, "LH*/",        1, lhscale,         lhscale,         OP_VISIBLE) \
  X(186, "L+H",         1, bytesum,         bytesum,         OP_VISIBLE) \
  X(187, "L-H",         1, bytediff,        bytediff,        OP_VISIBLE) \
  X(188, "256*",        1, twofivesixstar,  twofivesixstar,  OP_VISIBLE) \
  X(189, "RANDOM",      1, random,          random,          OP_VISIBLE) \
  X(190, "XRES",        1, xres,            xres,            OP_VISIBLE) \
  X(191, "XY!",         1, xystore,         xystore,         OP_VISIBLE) \
  X(192, "GLYPH!",      1, setglyph,        setglyph,        OP_VISIBLE) \
  X(193, "TCOLOR!",     1, tsetcolor,       tsetcolor,       OP_VISIBLE) \
  X(194, "CGS",         1, cgs,             cgs,             OP_VISIBLE) \
  X(195, "COLOR",       1, getcolor,        getcolor,        OP_VISIBLE) \
  X(196, "COLOR!",      1, setcolor,        setcolor,        OP_VISIBLE) \
  X(197, "HCOLOR!",     1, hsetcolor,       hsetcolor,       OP_VISIBLE) \
  X(198, "MCOLOR!",     1, msetcolor,       msetcolor,       OP_VISIBLE) \
  X(199, "LCOLOR!",     1, lsetcolor,       lsetcolor,       OP_VISIBLE) \
  X(200, "BCOLOR!",     1, bsetcolor,       bsetcolor,       OP_VISIBLE) \
  X(201, "CLEAR",       1, clear,           clear,           OP_VISIBLE) \
  X(202, "HCLEAR",      1, hclear,          hclear,          OP_VISIBLE) \
  X(203, "MCLEAR",      1, mclear,          mclear,          OP_VISIBLE) \
  X(204, "LCLEAR",      1, lclear,          lclear,          OP_VISIBLE) \
  X(205, "BCLEAR",      1, bclear,          bclear,          OP_VISIBLE) \
  X(206, "PLOT",        1, plot,            plot,            OP_VISIBLE) \
  X(207, "HPLOT",       1, hplot,           hplot,           OP_VISIBLE) \
  X(208, "MPLOT",       1, mplot,           mplot,           OP_VISIBLE) \
  X(209, "LPLOT",       1, lplot,           lplot,           OP_VISIBLE) \
  X(210, "BPLOT",       1, bplot,           bplot,           OP_VISIBLE) \
  X(211, "PSET",        1, pset,            pset,            OP_VISIBLE) \
  X(212, "MPSET",       1, mpset,           mpset,           OP_VISIBLE) \
  X(213, "LPSET",       1, lpset,           lpset,           OP_VISIBLE) \
  X(214, "BPSET",       1, bpset,           bpset,           OP_VISIBLE) \
  X(215, "VLIN",        1, vlin,            vlin,            OP_VISIBLE) \
  X(216, "HVLIN",       1, hvlin,           hvlin,           OP_VISIBLE) \
  X(217, "MVLIN",       1, mvlin,           mvlin,           OP_VISIBLE) \
  X(218, "LVLIN",       1, lvlin,           lvlin,           OP_VISIBLE) \
  X(219, "BVLIN",       1, bvlin,           bvlin,           OP_VISIBLE) \
  X(220, "HLIN",        1, hlin,            hlin,            OP_VISIBLE) \
  X(221, "HHLIN",       1, hhlin,           hhlin,           OP_VISIBLE) \
  X(222, "MHLIN",       1, mhlin,           mhlin,           OP_VISIBLE) \
  X(223, "LHLIN",       1, lhlin,           lhlin,           OP_VISIBLE) \
  X(224, "BHLIN",       1, bhlin,           bhlin,           OP_VISIBLE) \
  X(225, "RECT",        1, rect,            rect,            OP_VISIBLE) \
  X(226, "HRECT",       1, hrect,           hrect,           OP_VISIBLE) \
  X(227, "MRECT",       1, mrect,           mrect,           OP_VISIBLE) \
  X(228, "LRECT",       1, lrect,           lrect,           OP_VISIBLE) \
  X(229, "BRECT",       1, brect,           brect,           OP_VISIBLE) \
  X(230, "LINE",        1, line,            line,            OP_VISIBLE) \
  X(231, "HLINE",       1, hline,           hline,           OP_VISIBLE) \
  X(232, "MLINE",       1, mline,           mline,           OP_VISIBLE) \
  X(233, "LLINE",       1, lline,           lline,           OP_VISIBLE) \
  X(234, "BLINE",       1, bline,           bline,           OP_VISIBLE) \
  X(235, "COS",         1, cos,             cos,             OP_VISIBLE) \
  X(236, "SIN",         1, sin,             sin,             OP_VISIBLE) \
  X(237, "CCOS",        1, ccos,            ccos,            OP_VISIBLE) \
  X(238, "CSIN",        1, csin,            csin,            OP_VISIBLE) \
  X(239, "LH-COS/SIN",  1, lhcossin,        lhcossin,        OP_VISIBLE) \
  X(240, "HSV",         1, hsv,             hsv,             OP_VISIBLE) \
  X(241, "VSYNC",       1, vsync,           vsync,           OP_VISIBLE) \
  X(242, "",            1, invalid,         invalid,         0) \
  X(243, "",            1, invalid,         invalid_f3,      0) \
  X(244, "",            1, invalid,         invalid_f4,      0) \
  X(245, "",            1, invalid,         invalid_f5,      0) \
  X(246, "",            1, invalid,         invalid_f6,      0) \
  X(247, "",            1, invalid,         invalid_f7,      0) \
  X(248, "",            1, invalid,         invalid_f8,      0) \
  X(249, "",            1, invalid,         invalid_f9,      0) \
  X(250, "",            1, invalid,         invalid_fa,      0) \
  X(251, "",            1, invalid,         invalid_fb,      0) \
  X(252, "",            1, invalid,         invalid_fc,      0) \
  X(253, "",            1, invalid,         invalid_fd,      0) \
  X(254, "(EXIT)",      1, endword,         endword,         OP_COMPILEONLY) \
  X(255, "(EXT)",      2I, ext,             ext,             OP_COMPILEONLY)


#define EXT_OPCODES \
  X(0,   "?DUP",        1, qdup,            qdup,            OP_VISIBLE) \
  X(1,   "-ROT",        1, nrot,            nrot,            OP_VISIBLE) \
  X(2,   "TUCK",        1, tuck,            tuck,            OP_VISIBLE) \
  X(3,   "PICK",        1, pick,            pick,            OP_VISIBLE) \
  X(4,   "ROLL",        1, roll,            roll,            OP_VISIBLE) \
  X(5,   "2SWAP",       1, twoswap,         twoswap,         OP_VISIBLE) \
  X(6,   "2OVER",       1, twoover,         twoover,         OP_VISIBLE) \
  X(7,   "2ROT",        1, tworot,          tworot,          OP_VISIBLE) \
  X(8,   "2SELECT",     1, twoselect,       twoselect,       OP_VISIBLE) \
  X(9,   "C>S",         1, signextend,      signextend,      OP_VISIBLE) \
  X(10,  "MIN",         1, min,             min,             OP_VISIBLE) \
  X(11,  "MAX",         1, max,             max,             OP_VISIBLE) \
  X(12,  "UMIN",        1, umin,            umin,            OP_VISIBLE) \
  X(13,  "UMAX",        1, umax,            umax,            OP_VISIBLE) \
  X(14,  "WITHIN",      1, within,          within,          OP_VISIBLE) \
  X(15,  "LSHIFT",      1, lshift,          lshift,          OP_VISIBLE) \
  X(16,  "RSHIFT",      1, rshift,          rshift,          OP_VISIBLE) \
  X(17,  "ARSHIFT",     1, arshift,         arshift,         OP_VISIBLE) \
  X(18,  "C+!",         1, caddstore,       caddstore,       OP_VISIBLE) /**/ \
  X(19,  "CBIT@",       1, cbittest,        cbittest,        OP_VISIBLE) /**/ \
  X(20,  "CBIC!",       1, cbitclear,       cbitclear,       OP_VISIBLE) /**/ \
  X(21,  "CBIS!",       1, cbitset,         cbitset,         OP_VISIBLE) /**/ \
  X(22,  "CXOR!",       1, cbitflip,        cbitflip,        OP_VISIBLE) /**/ \
  X(23,  "S>D",         1, stod,            stod,            OP_VISIBLE) \
  X(24,  "D=",          1, deq,             deq,             OP_VISIBLE) \
  X(25,  "D<>",         1, dne,             dne,             OP_VISIBLE) \
  X(26,  "D0=",         1, deq0,            deq0,            OP_VISIBLE) \
  X(27,  "D0<>",        1, dne0,            dne0,            OP_VISIBLE) \
  X(28,  "D0<",         1, dlt0,            dlt0,            OP_VISIBLE) \
  X(29,  "D0<=",        1, dle0,            dle0,            OP_VISIBLE) \
  X(30,  "D0>",         1, dgt0,            dgt0,            OP_VISIBLE) \
  X(31,  "D0>=",        1, dge0,            dge0,            OP_VISIBLE) \
  X(32,  "D<",          1, dlt,             dlt,             OP_VISIBLE) \
  X(33,  "D<=",         1, dle,             dle,             OP_VISIBLE) \
  X(34,  "D>",          1, dgt,             dgt,             OP_VISIBLE) \
  X(35,  "D>=",         1, dge,             dge,             OP_VISIBLE) \
  X(36,  "DU<",         1, dult,            dult,            OP_VISIBLE) \
  X(37,  "DU<=",        1, dule,            dule,            OP_VISIBLE) \
  X(38,  "DU>",         1, dugt,            dugt,            OP_VISIBLE) \
  X(39,  "DU>=",        1, duge,            duge,            OP_VISIBLE) \
  X(40,  "D2*",         1, dtwostar,        dtwostar,        OP_VISIBLE) \
  X(41,  "D2/",         1, dtwoslash,       dtwoslash,       OP_VISIBLE) \
  X(42,  "DU2/",        1, dutwoslash,      dutwoslash,      OP_VISIBLE) \
  X(43,  "DABS",        1, dabs,            dabs,            OP_VISIBLE) \
  X(44,  "DNEGATE",     1, dnegate,         dnegate,         OP_VISIBLE) \
  X(45,  "DMIN",        1, dmin,            dmin,            OP_VISIBLE) \
  X(46,  "DMAX",        1, dmax,            dmax,            OP_VISIBLE) \
  X(47,  "M+",          1, mplus,           mplus,           OP_VISIBLE) \
  X(48,  "D+",          1, dplus,           dplus,           OP_VISIBLE) \
  X(49,  "D-",          1, dminus,          dminus,          OP_VISIBLE) \
  X(50,  "*",           1, star,            star,            OP_VISIBLE) \
  X(51,  "*/",          1, starslash,       starslash,       OP_VISIBLE) \
  X(52,  "*/MOD",       1, starslashmod,    starslashmod,    OP_VISIBLE) \
  X(53,  "/MOD",        1, slashmod,        slashmod,        OP_VISIBLE) \
  X(54,  "U/MOD",       1, uslashmod,       uslashmod,       OP_VISIBLE) \
  X(55,  "UM/MOD",      1, umslashmod,      umslashmod,      OP_VISIBLE) \
  X(56,  "FM/MOD",      1, fmslashmod,      fmslashmod,      OP_VISIBLE) \
  X(57,  "SM/REM",      1, smslashrem,      smslashrem,      OP_VISIBLE) \
  X(58,  "/",           1, slash,           slash,           OP_VISIBLE) \
  X(59,  "MOD",         1, mod,             mod,             OP_VISIBLE) \
  X(60,  "U/",          1, uslash,          uslash,          OP_VISIBLE) \
  X(61,  "UMOD",        1, umod,            umod,            OP_VISIBLE) \
  X(62,  "M*/",         1, mstarslash,      mstarslash,      OP_VISIBLE) \
  X(63,  "UM*/",        1, umstarslash,     umstarslash,     OP_VISIBLE) \
  X(64,  "M*",          1, mstar,           mstar,           OP_VISIBLE) \
  X(65,  "UM*",         1, umstar,          umstar,          OP_VISIBLE) \
  X(66,  ">RESOLVE",    1, resolve,         resolve,         OP_VISIBLE) \
  X(67,  "<RESOLVE",    1, bresolve,        bresolve,        OP_VISIBLE) \
  X(68,  "(RAKE)",      1, rake,            rake,            OP_COMPILEONLY) \
  X(69,  "EXIT",        1, exit,            exit,            OP_VISIBLE) \
  X(70,  ";",           1, semicolon,       semicolon,       OP_COMPILEONLY|OP_VISIBLE) \
  X(71,  "];",          1, execsemicolon,   execsemicolon,   OP_COMPILEONLY|OP_VISIBLE) \
  X(72,  "CH.",         1, chdot,           chdot,           OP_VISIBLE) \
  X(73,  "C>HEX",       1, ctohex,          ctohex,          OP_VISIBLE) \
  X(74,  "U>HEX",       1, utohex,          utohex,          OP_VISIBLE) \
  X(75,  ".",           1, dot,             dot,             OP_VISIBLE) /**/ \
  X(76,  "U.",          1, udot,            udot,            OP_VISIBLE) /**/ \
  X(77,  "D.",          1, ddot,            ddot,            OP_VISIBLE) \
  X(78,  "UD.",         1, uddot,           uddot,           OP_VISIBLE) \
  X(79,  ".R",          1, dotr,            dotr,            OP_VISIBLE) \
  X(80,  "U.R",         1, udotr,           udotr,           OP_VISIBLE) \
  X(81,  "D.R",         1, ddotr,           ddotr,           OP_VISIBLE) \
  X(82,  "LH.",         1, lhdot,           lhdot,           OP_VISIBLE) \
  X(83,  "SIGN",        1, sign,            sign,            OP_VISIBLE) \
  X(84,  "#S",          1, nums,            nums,            OP_VISIBLE) \
  X(85,  "HOLDS",       1, holds,           holds,           OP_VISIBLE) \
  X(86,  "#>",          1, numend,          numend,          OP_VISIBLE) \
  X(87,  ">MARKER",     1, tomarker,        tomarker,        OP_VISIBLE) \
  X(88,  "MARKER>",     1, markerfrom,      markerfrom,      OP_VISIBLE) \
  X(89,  "MARKER,",     1, markercomma,     markercomma,     OP_VISIBLE) \
  X(90,  "EMPTY",       1, empty,           empty,           OP_VISIBLE) \
  X(91,  "SHRED",       1, shred,           shred,           OP_VISIBLE) \
  X(92,  "SPACES",      1, spaces,          spaces,          OP_VISIBLE) \
  X(93,  "KEY?",        1, keyq,            keyq,            OP_VISIBLE) \
  X(94,  "ACCEPT",      1, accept,          accept,          OP_VISIBLE) \
  X(95,  "REFILL",      1, refill,          refill,          OP_VISIBLE) \
  X(96,  "EVALUATE",    1, evaluate,        evaluate,        OP_VISIBLE) \
  X(97,  ":NONAME",     1, noname,          noname,          OP_VISIBLE) \
  X(98,  "COMP'",       1, comptick,        comptick,        OP_VISIBLE) \
  X(99,  "FIND",        1, find,            find,            OP_VISIBLE) \
  X(100, "FIND-NAME",   1, findname,        findname,        OP_VISIBLE) \
  X(101, "FORGET-NAME", 1, forgetname,      forgetname,      OP_VISIBLE) \
  X(102, ">BODY",       1, tobody,          tobody,          OP_VISIBLE) \
  X(103, "SP@",         1, spfetch,         spfetch,         OP_VISIBLE) \
  X(104, "SP!",         1, spstore,         spstore,         OP_VISIBLE) \
  X(105, "DEPTH",       1, depth,           depth,           OP_VISIBLE) \
  X(106, "UNUSED",      1, unused,          unused,          OP_VISIBLE) \
  X(107, "PARSE\"",     1, parsequote,      parsequote,      OP_VISIBLE) \
  X(108, "PARSE\\\"",   1, parseslashquote, parseslashquote, OP_VISIBLE) \
  X(109, "WORD",        1, word,            word,            OP_VISIBLE) \
  X(110, "PLACE",       1, place,           place,           OP_VISIBLE) \
  X(111, ">NUMBER",     1, tonumber,        tonumber,        OP_VISIBLE) \
  X(112, "CMOVE",       1, cmove,           cmove,           OP_VISIBLE) /**/ \
  X(113, "CMOVE>",      1, cmoveup,         cmoveup,         OP_VISIBLE) /**/ \
  X(114, "MOVE",        1, move,            move,            OP_VISIBLE) /**/ \
  X(115, "-TRAILING",   1, dashtrailing,    dashtrailing,    OP_VISIBLE) \
  X(116, "/STRING",     1, slashstring,     slashstring,     OP_VISIBLE) \
  X(117, "COMPARE",     1, compare,         compare,         OP_VISIBLE) \
  X(118, "SEARCH",      1, search,          search,          OP_VISIBLE) \
  X(119, "ENVIRONMENT?",1, environmentq,    environmentq,    OP_VISIBLE) \
  X(120, "SOURCE",      1, source,          source,          OP_VISIBLE) \
  X(121, "CATCH",       1, catch,           catch,           OP_VISIBLE) \
  X(122, "DELAY",       1, delay,           delay,           OP_VISIBLE) \
  X(123, "TEXT",        1, fortycol,        fortycol,        OP_VISIBLE) \
  X(124, "HTEXT" ,      1, eightycol,       eightycol,       OP_VISIBLE) \
  X(125, "+COLOR",      1, coloron,         coloron,         OP_VISIBLE) \
  X(126, "-COLOR",      1, coloroff,        coloroff,        OP_VISIBLE) \
  X(127, "CTEXT",       1, colorfortycol,   colorfortycol,   OP_VISIBLE) \
  X(128, "CLS",         1, cls,             cls,             OP_VISIBLE) \
  X(129, "FORM",        1, form,            form,            OP_VISIBLE) \
  X(130, "AT-XY",       1, atxy,            atxy,            OP_VISIBLE) \
  X(131, "CURSOR@",     1, getcursor,       getcursor,       OP_VISIBLE) \
  X(132, "CURSOR!",     1, setcursor,       setcursor,       OP_VISIBLE) \
  X(133, "CURSOR+!",    1, movecursor,      movecursor,      OP_VISIBLE) \
  X(134, "WINDOW",      1, window,          window,          OP_VISIBLE) \
  X(135, "XY>ADDR",     1, xytoaddr,        xytoaddr,        OP_VISIBLE) \
  X(136, "TSETCC",      1, tsetcc,          tsetcc,          OP_VISIBLE) \
  X(137, "TBOX",        1, tbox,            tbox,            OP_VISIBLE) \
  X(138, "TRECTC",      1, trectc,          trectc,          OP_VISIBLE) \
  X(139, "TRECT",       1, trect,           trect,           OP_VISIBLE) \
  X(140, "TRECTCC",     1, trectcc,         trectcc,         OP_VISIBLE) \
  X(141, "TSPRITE",     1, tsprite,         tsprite,         OP_VISIBLE) \
  X(142, "TSPRITECC",   1, tspritecc,       tspritecc,       OP_VISIBLE) \
  X(143, "GLYPH@",      1, getglyph,        getglyph,        OP_VISIBLE) \
  X(144, "(FONT:)",     1, makefont,        makefont,        OP_COMPILEONLY) \
  X(145, "GMODE",       1, gmode,           gmode,           OP_VISIBLE) \
  X(146, "GSPLIT",      1, gsplit,          gsplit,          OP_VISIBLE) \
  X(147, "LPAL!",       1, lpalette,        lpalette,        OP_VISIBLE) \
  X(148, "EEC@",        1, eecfetch,        eecfetch,        OP_VISIBLE) \
  X(149, "EE@",         1, eefetch,         eefetch,         OP_VISIBLE) \
  X(150, "EEC!",        1, eecstore,        eecstore,        OP_VISIBLE) \
  X(151, "EE!",         1, eestore,         eestore,         OP_VISIBLE) \
  X(152, "RAM>EE",      1, ramtoee,         ramtoee,         OP_VISIBLE) \
  X(153, "EE>RAM",      1, eetoram,         eetoram,         OP_VISIBLE) \
  X(154, "EEFILL",      1, eefill,          eefill,          OP_VISIBLE) \
  X(155, "EETYPE",      1, eetype,          eetype,          OP_VISIBLE) \
  X(156, "BCD+",        1, bcdplus,         bcdplus,         OP_VISIBLE) \
  X(157, "BCD-",        1, bcdminus,        bcdminus,        OP_VISIBLE) \
  X(158, "DBCD+",       1, dbcdplus,        dbcdplus,        OP_VISIBLE) \
  X(159, "DBCD-",       1, dbcdminus,       dbcdminus,       OP_VISIBLE) \
  X(160, "MBCD+",       1, mbcdplus,        mbcdplus,        OP_VISIBLE) \
  X(161, "LOAD",        1, load,            load,            OP_VISIBLE) \
  X(162, "SPI0",        1, spi0,            spi0,            OP_VISIBLE) \
  X(163, "CSPI0",       1, cspi0,           cspi0,           OP_VISIBLE) \
  X(164, "SPI1",        1, spi1,            spi1,            OP_VISIBLE) \
  X(165, "CSPI1",       1, cspi1,           cspi1,           OP_VISIBLE) \
  X(166, "SPI2",        1, spi2,            spi2,            OP_VISIBLE) \
  X(167, "CSPI2",       1, cspi2,           cspi2,           OP_VISIBLE) \
  X(168, "SPI3",        1, spi3,            spi3,            OP_VISIBLE) \
  X(169, "CSPI3",       1, cspi3,           cspi3,           OP_VISIBLE) \
  X(170, "(ABORT\")",   1, abortq,          abortq,          OP_COMPILEONLY) \
  X(171, "(.S)",        1, dumpstack,       dumpstack,       0) \
  X(172, "BYE",         1, bye,             bye,             OP_VISIBLE) \
  X(173, "TINY",        1, tiny,            tiny,            OP_VISIBLE) \
  X(174, "-TINY",       1, exittiny,        exittiny,        OP_VISIBLE) \
  X(175, "BEEP",        1, beep,            beep,            OP_VISIBLE) \
  X(176, "PTONE",       1, ptonesync,       ptonesync,       OP_VISIBLE) \
  X(177, "TONE",        1, tonesync,        tonesync,        OP_VISIBLE) \
  X(178, "PTONE!",      1, ptone,           ptone,           OP_VISIBLE) \
  X(179, "TONE!",       1, tone,            tone,            OP_VISIBLE) \
  X(180, "PNOTE",       1, pnotesync,       pnotesync,       OP_VISIBLE) \
  X(181, "NOTE",        1, notesync,        notesync,        OP_VISIBLE) \
  X(182, "PNOTE!",      1, pnote,           pnote,           OP_VISIBLE) \
  X(183, "NOTE!",       1, note,            note,            OP_VISIBLE) \
  X(184, "+SOUND",      1, soundon,         soundon,         OP_VISIBLE) \
  X(185, "-SOUND",      1, soundoff,        soundoff,        OP_VISIBLE) \
  X(186, "+INT",        1, sei,             sei,             OP_VISIBLE) \
  X(187, "-INT",        1, cli,             cli,             OP_VISIBLE) \
  X(188, "XMSTAT",      1, xmgetstatus,     xmgetstatus,     OP_VISIBLE) \
  X(189, "XMSTAT!",     1, xmsetstatus,     xmsetstatus,     OP_VISIBLE) \
  X(190, "XMC@",        1, xmcfetch,        xmcfetch,        OP_VISIBLE) \
  X(191, "XMC!",        1, xmcstore,        xmcstore,        OP_VISIBLE) \
  X(192, "RAM>XM",      1, ramtoxm,         ramtoxm,         OP_VISIBLE) \
  X(193, "XM>RAM",      1, xmtoram,         xmtoram,         OP_VISIBLE) \
  X(194, "#BLOCKS",     1, numblocks,       numblocks,       OP_VISIBLE) \
  X(195, "#XM-BLOCKS",  1, numxmblocks,     numxmblocks,     OP_VISIBLE) \
  X(196, "COPY-BLOCKS", 1, copyblocks,      copyblocks,      OP_VISIBLE) \
  X(197, "FILL-BLOCKS", 1, fillblocks,      fillblocks,      OP_VISIBLE) \


#undef X
#ifdef __ASSEMBLER__
/* the GNU assembler recognizes $ as a statement separator */
#define X(opnum,str,props,asmid,enumid,visibility) .equ .opcode_##opnum, opcode_##asmid $ .equ OP_##enumid, (opnum) $
OPCODES
#undef X
#define X(opnum,str,props,asmid,enumid,visibility) .equ .ext_opcode_##opnum, ext_opcode_##asmid $ .equ OPX_##enumid, (opnum) $
EXT_OPCODES
#else
#define X(opnum,str,props,asmid,enumid,visibility) OP_##enumid = opnum,
enum {
  OPCODES
};
#undef X
#define X(opnum,str,props,asmid,enumid,visibility) OPX_##enumid = opnum,
enum {
  EXT_OPCODES
};
#endif

#endif
