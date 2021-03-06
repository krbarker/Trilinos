C    Copyright(C) 2014 National Technology & Engineering Solutions of
C    Sandia, LLC (NTESS).  Under the terms of Contract DE-NA0003525 with
C    NTESS, the U.S. Government retains certain rights in this software.
C    
C    Redistribution and use in source and binary forms, with or without
C    modification, are permitted provided that the following conditions are
C    met:
C    
C    * Redistributions of source code must retain the above copyright
C       notice, this list of conditions and the following disclaimer.
C    
C    * Redistributions in binary form must reproduce the above
C      copyright notice, this list of conditions and the following
C      disclaimer in the documentation and/or other materials provided
C      with the distribution.
C    
C    * Neither the name of NTESS nor the names of its
C      contributors may be used to endorse or promote products derived
C      from this software without specific prior written permission.
C    
C    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
C    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
C    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
C    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
C    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
C    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
C    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
C    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
C    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
C    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
C    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
C    

C $Id: inschm.f,v 1.1 1990/11/30 11:10:12 gdsjaar Exp $
C $Log: inschm.f,v $
C Revision 1.1  1990/11/30 11:10:12  gdsjaar
C Initial revision
C
C
CC* FILE: [.MAIN]INSCHM.FOR
CC* MODIFIED BY: TED BLACKER
CC* MODIFICATION DATE: 7/6/90
CC* MODIFICATION: COMPLETED HEADER INFORMATION
C
      SUBROUTINE INSCHM (MR, MSC, N8, N19, JJ, DUMMY, ISCHM, SCHEME,
     &   LINKSC, DEFSCH, NOROOM, DOLINK)
C***********************************************************************
C
C  SUBROUTINE INSCHM = INPUTS A SCHEME INTO THE DATABASE
C
C***********************************************************************
C
      DIMENSION ISCHM (MSC), SCHEME (MSC), LINKSC (2, MR)
C
      CHARACTER * 72 SCHEME, DEFSCH, DUMMY
C
      LOGICAL NOROOM, DOLINK, ADDLNK
C
      NOROOM = .TRUE.
      ADDLNK = .TRUE.
C
C  ENTER THE DEFAULT SCHEME IF THE REGION NUMBER IS ZERO
C
      IF (JJ .EQ. 0) THEN
         DEFSCH = DUMMY
C
C  ENTER THE SCHEME
C
      ELSE
         IF ( (DOLINK) .AND. (JJ .GT. N19))N19 = JJ
         N8 = N8 + 1
         J = N8
         IF (N8 .GT. MSC)RETURN
         ISCHM (J) = JJ
         IF (DOLINK)CALL LTSORT (MR, LINKSC, JJ, J, ADDLNK)
         SCHEME (J) = DUMMY
      ENDIF
      NOROOM = .FALSE.
      RETURN
C
      END
