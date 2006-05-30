! Fast Fourier/Cosine/Sine Transform
!     dimension   :one
!     data length :power of 2
!     decimation  :frequency
!     radix       :4, 2
!     data        :inplace
!     table       :use
! subroutines
!     cdft: Complex Discrete Fourier Transform
!     rdft: Real Discrete Fourier Transform
!     ddct: Discrete Cosine Transform
!     ddst: Discrete Sine Transform
!     dfct: Cosine Transform of RDFT (Real Symmetric DFT)
!     dfst: Sine Transform of RDFT (Real Anti-symmetric DFT)
!
!
! -------- Complex DFT (Discrete Fourier Transform) --------
!     [definition]
!         <case1>
!             X(k) = sum_j=0^n-1 x(j)*exp(2*pi*i*j*k/n), 0<=k<n
!         <case2>
!             X(k) = sum_j=0^n-1 x(j)*exp(-2*pi*i*j*k/n), 0<=k<n
!         (notes: sum_j=0^n-1 is a summation from j=0 to n-1)
!     [usage]
!         <case1>
!             ip(0) = 0  ! first time only
!             call cdft(2*n, 1, a, ip, w)
!         <case2>
!             ip(0) = 0  ! first time only
!             call cdft(2*n, -1, a, ip, w)
!     [parameters]
!         2*n          :data length (integer)
!                       n >= 1, n = power of 2
!         a(0:2*n-1)   :input/output data (real*8)
!                       input data
!                           a(2*j) = Re(x(j)), 
!                           a(2*j+1) = Im(x(j)), 0<=j<n
!                       output data
!                           a(2*k) = Re(X(k)), 
!                           a(2*k+1) = Im(X(k)), 0<=k<n
!         ip(0:*)      :work area for bit reversal (integer)
!                       length of ip >= 2+sqrt(n)
!                       strictly, 
!                       length of ip >= 
!                           2+2**(int(log(n+0.5)/log(2.0))/2).
!                       ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:n/2-1)   :cos/sin table (real*8)
!                       w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             call cdft(2*n, -1, a, ip, w)
!         is 
!             call cdft(2*n, 1, a, ip, w)
!             do j = 0, 2 * n - 1
!                 a(j) = a(j) / n
!             end do
!         .
!
!
! -------- Real DFT / Inverse of Real DFT --------
!     [definition]
!         <case1> RDFT
!             R(k) = sum_j=0^n-1 a(j)*cos(2*pi*j*k/n), 0<=k<=n/2
!             I(k) = sum_j=0^n-1 a(j)*sin(2*pi*j*k/n), 0<k<n/2
!         <case2> IRDFT (excluding scale)
!             a(k) = (R(0) + R(n/2)*cos(pi*k))/2 + 
!                    sum_j=1^n/2-1 R(j)*cos(2*pi*j*k/n) + 
!                    sum_j=1^n/2-1 I(j)*sin(2*pi*j*k/n), 0<=k<n
!     [usage]
!         <case1>
!             ip(0) = 0  ! first time only
!             call rdft(n, 1, a, ip, w)
!         <case2>
!             ip(0) = 0  ! first time only
!             call rdft(n, -1, a, ip, w)
!     [parameters]
!         n            :data length (integer)
!                       n >= 2, n = power of 2
!         a(0:n-1)     :input/output data (real*8)
!                       <case1>
!                           output data
!                               a(2*k) = R(k), 0<=k<n/2
!                               a(2*k+1) = I(k), 0<k<n/2
!                               a(1) = R(n/2)
!                       <case2>
!                           input data
!                               a(2*j) = R(j), 0<=j<n/2
!                               a(2*j+1) = I(j), 0<j<n/2
!                               a(1) = R(n/2)
!         ip(0:*)      :work area for bit reversal (integer)
!                       length of ip >= 2+sqrt(n/2)
!                       strictly, 
!                       length of ip >= 
!                           2+2**(int(log(n/2+0.5)/log(2.0))/2).
!                       ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:n/2-1)   :cos/sin table (real*8)
!                       w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             call rdft(n, 1, a, ip, w)
!         is 
!             call rdft(n, -1, a, ip, w)
!             do j = 0, n - 1
!                 a(j) = a(j) * 2 / n
!             end do
!         .
!
!
! -------- DCT (Discrete Cosine Transform) / Inverse of DCT --------
!     [definition]
!         <case1> IDCT (excluding scale)
!             C(k) = sum_j=0^n-1 a(j)*cos(pi*j*(k+1/2)/n), 0<=k<n
!         <case2> DCT
!             C(k) = sum_j=0^n-1 a(j)*cos(pi*(j+1/2)*k/n), 0<=k<n
!     [usage]
!         <case1>
!             ip(0) = 0  ! first time only
!             call ddct(n, 1, a, ip, w)
!         <case2>
!             ip(0) = 0  ! first time only
!             call ddct(n, -1, a, ip, w)
!     [parameters]
!         n            :data length (integer)
!                       n >= 2, n = power of 2
!         a(0:n-1)     :input/output data (real*8)
!                       output data
!                           a(k) = C(k), 0<=k<n
!         ip(0:*)      :work area for bit reversal (integer)
!                       length of ip >= 2+sqrt(n/2)
!                       strictly, 
!                       length of ip >= 
!                           2+2**(int(log(n/2+0.5)/log(2.0))/2).
!                       ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:n*5/4-1) :cos/sin table (real*8)
!                       w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             call ddct(n, -1, a, ip, w)
!         is 
!             a(0) = a(0) / 2
!             call ddct(n, 1, a, ip, w)
!             do j = 0, n - 1
!                 a(j) = a(j) * 2 / n
!             end do
!         .
!
!
! -------- DST (Discrete Sine Transform) / Inverse of DST --------
!     [definition]
!         <case1> IDST (excluding scale)
!             S(k) = sum_j=1^n A(j)*sin(pi*j*(k+1/2)/n), 0<=k<n
!         <case2> DST
!             S(k) = sum_j=0^n-1 a(j)*sin(pi*(j+1/2)*k/n), 0<k<=n
!     [usage]
!         <case1>
!             ip(0) = 0  ! first time only
!             call ddst(n, 1, a, ip, w)
!         <case2>
!             ip(0) = 0  ! first time only
!             call ddst(n, -1, a, ip, w)
!     [parameters]
!         n            :data length (integer)
!                       n >= 2, n = power of 2
!         a(0:n-1)     :input/output data (real*8)
!                       <case1>
!                           input data
!                               a(j) = A(j), 0<j<n
!                               a(0) = A(n)
!                           output data
!                               a(k) = S(k), 0<=k<n
!                       <case2>
!                           output data
!                               a(k) = S(k), 0<k<n
!                               a(0) = S(n)
!         ip(0:*)      :work area for bit reversal (integer)
!                       length of ip >= 2+sqrt(n/2)
!                       strictly, 
!                       length of ip >= 
!                           2+2**(int(log(n/2+0.5)/log(2.0))/2).
!                       ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:n*5/4-1) :cos/sin table (real*8)
!                       w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             call ddst(n, -1, a, ip, w)
!         is 
!             a(0) = a(0) / 2
!             call ddst(n, 1, a, ip, w)
!             do j = 0, n - 1
!                 a(j) = a(j) * 2 / n
!             end do
!         .
!
!
! -------- Cosine Transform of RDFT (Real Symmetric DFT) --------
!     [definition]
!         C(k) = sum_j=0^n a(j)*cos(pi*j*k/n), 0<=k<=n
!     [usage]
!         ip(0) = 0  ! first time only
!         call dfct(n, a, t, ip, w)
!     [parameters]
!         n            :data length - 1 (integer)
!                       n >= 2, n = power of 2
!         a(0:n)       :input/output data (real*8)
!                       output data
!                           a(k) = C(k), 0<=k<=n
!         t(0:n/2)     :work area (real*8)
!         ip(0:*)      :work area for bit reversal (integer)
!                       length of ip >= 2+sqrt(n/4)
!                       strictly, 
!                       length of ip >= 
!                           2+2**(int(log(n/4+0.5)/log(2.0))/2).
!                       ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:n*5/8-1) :cos/sin table (real*8)
!                       w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             a(0) = a(0) / 2
!             a(n) = a(n) / 2
!             call dfct(n, a, t, ip, w)
!         is 
!             a(0) = a(0) / 2
!             a(n) = a(n) / 2
!             call dfct(n, a, t, ip, w)
!             do j = 0, n
!                 a(j) = a(j) * 2 / n
!             end do
!         .
!
!
! -------- Sine Transform of RDFT (Real Anti-symmetric DFT) --------
!     [definition]
!         S(k) = sum_j=1^n-1 a(j)*sin(pi*j*k/n), 0<k<n
!     [usage]
!         ip(0) = 0  ! first time only
!         call dfst(n, a, t, ip, w)
!     [parameters]
!         n            :data length + 1 (integer)
!                       n >= 2, n = power of 2
!         a(0:n-1)     :input/output data (real*8)
!                       output data
!                           a(k) = S(k), 0<k<n
!                       (a(0) is used for work area)
!         t(0:n/2-1)   :work area (real*8)
!         ip(0:*)      :work area for bit reversal (integer)
!                       length of ip >= 2+sqrt(n/4)
!                       strictly, 
!                       length of ip >= 
!                           2+2**(int(log(n/4+0.5)/log(2.0))/2).
!                       ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:n*5/8-1) :cos/sin table (real*8)
!                       w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             call dfst(n, a, t, ip, w)
!         is 
!             call dfst(n, a, t, ip, w)
!             do j = 1, n - 1
!                 a(j) = a(j) * 2 / n
!             end do
!         .
!
!
! Appendix :
!     The cos/sin table is recalculated when the larger table required.
!     w() and ip() are compatible with all routines.
!
!
      subroutine cdft(n, isgn, a, ip, w)
      integer n, isgn, ip(0 : *), j
      real*8 a(0 : n - 1), w(0 : *)
      if (n .gt. 4 * ip(0)) then
          call makewt(n / 4, ip, w)
      end if
      if (n .gt. 4) call bitrv2(n, ip(2), a)
      if (n .gt. 4 .and. isgn .lt. 0) then
          do j = 1, n - 1, 2
              a(j) = -a(j)
          end do
          call cftsub(n, a, w)
          do j = 1, n - 1, 2
              a(j) = -a(j)
          end do
      else
          call cftsub(n, a, w)
      end if
      end
!
      subroutine rdft(n, isgn, a, ip, w)
      integer n, isgn, ip(0 : *), j, nw, nc
      real*8 a(0 : n - 1), w(0 : *), xi
      nw = ip(0)
      if (n .gt. 4 * nw) then
          nw = n / 4
          call makewt(nw, ip, w)
      end if
      nc = ip(1)
      if (n .gt. 4 * nc) then
          nc = n / 4
          call makect(nc, ip, w(nw))
      end if
      if (isgn .lt. 0) then
          a(1) = 0.5d0 * (a(1) - a(0))
          a(0) = a(0) + a(1)
          do j = 3, n - 1, 2
              a(j) = -a(j)
          end do
          if (n .gt. 4) then
              call rftsub(n, a, nc, w(nw))
              call bitrv2(n, ip(2), a)
          end if
          call cftsub(n, a, w)
          do j = 1, n - 1, 2
              a(j) = -a(j)
          end do
      else
          if (n .gt. 4) call bitrv2(n, ip(2), a)
          call cftsub(n, a, w)
          if (n .gt. 4) call rftsub(n, a, nc, w(nw))
          xi = a(0) - a(1)
          a(0) = a(0) + a(1)
          a(1) = xi
      end if
      end
!
      subroutine ddct(n, isgn, a, ip, w)
      integer n, isgn, ip(0 : *), j, nw, nc
      real*8 a(0 : n - 1), w(0 : *), xr
      nw = ip(0)
      if (n .gt. 4 * nw) then
          nw = n / 4
          call makewt(nw, ip, w)
      end if
      nc = ip(1)
      if (n .gt. nc) then
          nc = n
          call makect(nc, ip, w(nw))
      end if
      if (isgn .lt. 0) then
          xr = a(n - 1)
          do j = n - 2, 2, -2
              a(j + 1) = a(j - 1) - a(j)
              a(j) = a(j) + a(j - 1)
          end do
          a(1) = xr - a(0)
          a(0) = a(0) + xr
          if (n .gt. 4) then
              call rftsub(n, a, nc, w(nw))
              call bitrv2(n, ip(2), a)
          end if
          call cftsub(n, a, w)
          do j = 1, n - 1, 2
              a(j) = -a(j)
          end do
      end if
      call dctsub(n, a, nc, w(nw))
      if (isgn .ge. 0) then
          if (n .gt. 4) call bitrv2(n, ip(2), a)
          call cftsub(n, a, w)
          if (n .gt. 4) call rftsub(n, a, nc, w(nw))
          xr = a(0) - a(1)
          a(0) = a(0) + a(1)
          do j = 2, n - 2, 2
              a(j - 1) = a(j) - a(j + 1)
              a(j) = a(j) + a(j + 1)
          end do
          a(n - 1) = xr
      end if
      end
!
      subroutine ddst(n, isgn, a, ip, w)
      integer n, isgn, ip(0 : *), j, nw, nc
      real*8 a(0 : n - 1), w(0 : *), xr
      nw = ip(0)
      if (n .gt. 4 * nw) then
          nw = n / 4
          call makewt(nw, ip, w)
      end if
      nc = ip(1)
      if (n .gt. nc) then
          nc = n
          call makect(nc, ip, w(nw))
      end if
      if (isgn .lt. 0) then
          xr = a(n - 1)
          do j = n - 2, 2, -2
              a(j + 1) = a(j - 1) + a(j)
              a(j) = a(j) - a(j - 1)
          end do
          a(1) = -xr - a(0)
          a(0) = a(0) - xr
          if (n .gt. 4) then
              call rftsub(n, a, nc, w(nw))
              call bitrv2(n, ip(2), a)
          end if
          call cftsub(n, a, w)
          do j = 1, n - 1, 2
              a(j) = -a(j)
          end do
      end if
      call dstsub(n, a, nc, w(nw))
      if (isgn .ge. 0) then
          if (n .gt. 4) call bitrv2(n, ip(2), a)
          call cftsub(n, a, w)
          if (n .gt. 4) call rftsub(n, a, nc, w(nw))
          xr = a(0) - a(1)
          a(0) = a(0) + a(1)
          do j = 2, n - 2, 2
              a(j - 1) = -a(j) - a(j + 1)
              a(j) = a(j) - a(j + 1)
          end do
          a(n - 1) = -xr
      end if
      end
!
      subroutine dfct(n, a, t, ip, w)
      integer n, ip(0 : *), j, k, l, m, mh, nw, nc
      real*8 a(0 : n), t(0 : n / 2), w(0 : *), xr, xi
      nw = ip(0)
      if (n .gt. 8 * nw) then
          nw = n / 8
          call makewt(nw, ip, w)
      end if
      nc = ip(1)
      if (n .gt. 2 * nc) then
          nc = n / 2
          call makect(nc, ip, w(nw))
      end if
      m = n / 2
      xr = a(0) + a(n)
      a(0) = a(0) - a(n)
      t(0) = xr - a(m)
      t(m) = xr + a(m)
      if (n .gt. 2) then
          mh = m / 2
          do j = 1, mh - 1
              k = m - j
              xr = a(j) + a(n - j)
              a(j) = a(j) - a(n - j)
              xi = a(k) + a(n - k)
              a(k) = a(k) - a(n - k)
              t(j) = xr - xi
              t(k) = xr + xi
          end do
          t(mh) = a(mh) + a(n - mh)
          a(mh) = a(mh) - a(n - mh)
          call dctsub(m, a, nc, w(nw))
          if (m .gt. 4) call bitrv2(m, ip(2), a)
          call cftsub(m, a, w)
          if (m .gt. 4) call rftsub(m, a, nc, w(nw))
          xr = a(0) + a(1)
          a(n - 1) = a(0) - a(1)
          do j = m - 2, 2, -2
              a(2 * j + 1) = a(j) + a(j + 1)
              a(2 * j - 1) = a(j) - a(j + 1)
          end do
          a(1) = xr
          l = 2
          m = mh
          do while (m .ge. 2)
              call dctsub(m, t, nc, w(nw))
              if (m .gt. 4) call bitrv2(m, ip(2), t)
              call cftsub(m, t, w)
              if (m .gt. 4) call rftsub(m, t, nc, w(nw))
              a(n - l) = t(0) - t(1)
              a(l) = t(0) + t(1)
              k = 0
              do j = 2, m - 2, 2
                  k = k + 4 * l
                  a(k - l) = t(j) - t(j + 1)
                  a(k + l) = t(j) + t(j + 1)
              end do
              l = 2 * l
              mh = m / 2
              do j = 0, mh - 1
                  k = m - j
                  t(j) = t(m + k) - t(m + j)
                  t(k) = t(m + k) + t(m + j)
              end do
              t(mh) = t(m + mh)
              m = mh
          end do
          a(l) = t(0)
          a(n) = t(2) - t(1)
          a(0) = t(2) + t(1)
      else
          a(1) = a(0)
          a(2) = t(0)
          a(0) = t(1)
      end if
      end
!
      subroutine dfst(n, a, t, ip, w)
      integer n, ip(0 : *), j, k, l, m, mh, nw, nc
      real*8 a(0 : n - 1), t(0 : n / 2 - 1), w(0 : *), xr, xi
      nw = ip(0)
      if (n .gt. 8 * nw) then
          nw = n / 8
          call makewt(nw, ip, w)
      end if
      nc = ip(1)
      if (n .gt. 2 * nc) then
          nc = n / 2
          call makect(nc, ip, w(nw))
      end if
      if (n .gt. 2) then
          m = n / 2
          mh = m / 2
          do j = 1, mh - 1
              k = m - j
              xr = a(j) - a(n - j)
              a(j) = a(j) + a(n - j)
              xi = a(k) - a(n - k)
              a(k) = a(k) + a(n - k)
              t(j) = xr + xi
              t(k) = xr - xi
          end do
          t(0) = a(mh) - a(n - mh)
          a(mh) = a(mh) + a(n - mh)
          a(0) = a(m)
          call dstsub(m, a, nc, w(nw))
          if (m .gt. 4) call bitrv2(m, ip(2), a)
          call cftsub(m, a, w)
          if (m .gt. 4) call rftsub(m, a, nc, w(nw))
          xr = a(0) + a(1)
          a(n - 1) = a(1) - a(0)
          do j = m - 2, 2, -2
              a(2 * j + 1) = a(j) - a(j + 1)
              a(2 * j - 1) = -a(j) - a(j + 1)
          end do
          a(1) = xr
          l = 2
          m = mh
          do while (m .ge. 2)
              call dstsub(m, t, nc, w(nw))
              if (m .gt. 4) call bitrv2(m, ip(2), t)
              call cftsub(m, t, w)
              if (m .gt. 4) call rftsub(m, t, nc, w(nw))
              a(n - l) = t(1) - t(0)
              a(l) = t(0) + t(1)
              k = 0
              do j = 2, m - 2, 2
                  k = k + 4 * l
                  a(k - l) = -t(j) - t(j + 1)
                  a(k + l) = t(j) - t(j + 1)
              end do
              l = 2 * l
              mh = m / 2
              do j = 1, mh - 1
                  k = m - j
                  t(j) = t(m + k) + t(m + j)
                  t(k) = t(m + k) - t(m + j)
              end do
              t(0) = t(m + mh)
              m = mh
          end do
          a(l) = t(0)
      end if
      a(0) = 0
      end
!
! -------- initializing routines --------
!
      subroutine makewt(nw, ip, w)
      integer nw, ip(0 : *), nwh, j
      real*8 w(0 : nw - 1), delta, x, y
      ip(0) = nw
      ip(1) = 1
      if (nw .gt. 2) then
          nwh = nw / 2
          delta = atan(1.0d0) / nwh
          w(0) = 1
          w(1) = 0
          w(nwh) = cos(delta * nwh)
          w(nwh + 1) = w(nwh)
          do j = 2, nwh - 2, 2
              x = cos(delta * j)
              y = sin(delta * j)
              w(j) = x
              w(j + 1) = y
              w(nw - j) = y
              w(nw - j + 1) = x
          end do
          call bitrv2(nw, ip(2), w)
      end if
      end
!
      subroutine makect(nc, ip, c)
      integer nc, ip(0 : *), nch, j
      real*8 c(0 : nc - 1), delta
      ip(1) = nc
      if (nc .gt. 1) then
          nch = nc / 2
          delta = atan(1.0d0) / nch
          c(0) = 0.5d0
          c(nch) = 0.5d0 * cos(delta * nch)
          do j = 1, nch - 1
              c(j) = 0.5d0 * cos(delta * j)
              c(nc - j) = 0.5d0 * sin(delta * j)
          end do
      end if
      end
!
! -------- child routines --------
!
      subroutine bitrv2(n, ip, a)
      integer n, ip(0 : *), j, j1, k, k1, l, m, m2
      real*8 a(0 : n - 1), xr, xi
      ip(0) = 0
      l = n
      m = 1
      do while (4 * m .lt. l)
          l = l / 2
          do j = 0, m - 1
              ip(m + j) = ip(j) + l
          end do
          m = m * 2
      end do
      if (4 * m .gt. l) then
          do k = 1, m - 1
              do j = 0, k - 1
                  j1 = 2 * j + ip(k)
                  k1 = 2 * k + ip(j)
                  xr = a(j1)
                  xi = a(j1 + 1)
                  a(j1) = a(k1)
                  a(j1 + 1) = a(k1 + 1)
                  a(k1) = xr
                  a(k1 + 1) = xi
              end do
          end do
      else
          m2 = 2 * m
          do k = 1, m - 1
              do j = 0, k - 1
                  j1 = 2 * j + ip(k)
                  k1 = 2 * k + ip(j)
                  xr = a(j1)
                  xi = a(j1 + 1)
                  a(j1) = a(k1)
                  a(j1 + 1) = a(k1 + 1)
                  a(k1) = xr
                  a(k1 + 1) = xi
                  j1 = j1 + m2
                  k1 = k1 + m2
                  xr = a(j1)
                  xi = a(j1 + 1)
                  a(j1) = a(k1)
                  a(j1 + 1) = a(k1 + 1)
                  a(k1) = xr
                  a(k1 + 1) = xi
              end do
          end do
      end if
      end
!
      subroutine cftsub(n, a, w)
      integer n, j, j1, j2, j3, k, k1, ks, l, m
      real*8 a(0 : n - 1), w(0 : *)
      real*8 wk1r, wk1i, wk2r, wk2i, wk3r, wk3i
      real*8 x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i
      l = 2
      do while (2 * l .lt. n)
          m = 4 * l
          do j = 0, l - 2, 2
              j1 = j + l
              j2 = j1 + l
              j3 = j2 + l
              x0r = a(j) + a(j1)
              x0i = a(j + 1) + a(j1 + 1)
              x1r = a(j) - a(j1)
              x1i = a(j + 1) - a(j1 + 1)
              x2r = a(j2) + a(j3)
              x2i = a(j2 + 1) + a(j3 + 1)
              x3r = a(j2) - a(j3)
              x3i = a(j2 + 1) - a(j3 + 1)
              a(j) = x0r + x2r
              a(j + 1) = x0i + x2i
              a(j2) = x0r - x2r
              a(j2 + 1) = x0i - x2i
              a(j1) = x1r - x3i
              a(j1 + 1) = x1i + x3r
              a(j3) = x1r + x3i
              a(j3 + 1) = x1i - x3r
          end do
          if (m .lt. n) then
              wk1r = w(2)
              do j = m, l + m - 2, 2
                  j1 = j + l
                  j2 = j1 + l
                  j3 = j2 + l
                  x0r = a(j) + a(j1)
                  x0i = a(j + 1) + a(j1 + 1)
                  x1r = a(j) - a(j1)
                  x1i = a(j + 1) - a(j1 + 1)
                  x2r = a(j2) + a(j3)
                  x2i = a(j2 + 1) + a(j3 + 1)
                  x3r = a(j2) - a(j3)
                  x3i = a(j2 + 1) - a(j3 + 1)
                  a(j) = x0r + x2r
                  a(j + 1) = x0i + x2i
                  a(j2) = x2i - x0i
                  a(j2 + 1) = x0r - x2r
                  x0r = x1r - x3i
                  x0i = x1i + x3r
                  a(j1) = wk1r * (x0r - x0i)
                  a(j1 + 1) = wk1r * (x0r + x0i)
                  x0r = x3i + x1r
                  x0i = x3r - x1i
                  a(j3) = wk1r * (x0i - x0r)
                  a(j3 + 1) = wk1r * (x0i + x0r)
              end do
              k1 = 1
              ks = -1
              do k = 2 * m, n - m, m
                  k1 = k1 + 1
                  ks = -ks
                  wk1r = w(2 * k1)
                  wk1i = w(2 * k1 + 1)
                  wk2r = ks * w(k1)
                  wk2i = w(k1 + ks)
                  wk3r = wk1r - 2 * wk2i * wk1i
                  wk3i = 2 * wk2i * wk1r - wk1i
                  do j = k, l + k - 2, 2
                      j1 = j + l
                      j2 = j1 + l
                      j3 = j2 + l
                      x0r = a(j) + a(j1)
                      x0i = a(j + 1) + a(j1 + 1)
                      x1r = a(j) - a(j1)
                      x1i = a(j + 1) - a(j1 + 1)
                      x2r = a(j2) + a(j3)
                      x2i = a(j2 + 1) + a(j3 + 1)
                      x3r = a(j2) - a(j3)
                      x3i = a(j2 + 1) - a(j3 + 1)
                      a(j) = x0r + x2r
                      a(j + 1) = x0i + x2i
                      x0r = x0r - x2r
                      x0i = x0i - x2i
                      a(j2) = wk2r * x0r - wk2i * x0i
                      a(j2 + 1) = wk2r * x0i + wk2i * x0r
                      x0r = x1r - x3i
                      x0i = x1i + x3r
                      a(j1) = wk1r * x0r - wk1i * x0i
                      a(j1 + 1) = wk1r * x0i + wk1i * x0r
                      x0r = x1r + x3i
                      x0i = x1i - x3r
                      a(j3) = wk3r * x0r - wk3i * x0i
                      a(j3 + 1) = wk3r * x0i + wk3i * x0r
                  end do
              end do
          end if
          l = m
      end do
      if (l .lt. n) then
          do j = 0, l - 2, 2
              j1 = j + l
              x0r = a(j) - a(j1)
              x0i = a(j + 1) - a(j1 + 1)
              a(j) = a(j) + a(j1)
              a(j + 1) = a(j + 1) + a(j1 + 1)
              a(j1) = x0r
              a(j1 + 1) = x0i
          end do
      end if
      end
!
      subroutine rftsub(n, a, nc, c)
      integer n, nc, j, k, kk, ks
      real*8 a(0 : n - 1), c(0 : nc - 1), wkr, wki, xr, xi, yr, yi
      ks = 4 * nc / n
      kk = 0
      do k = n / 2 - 2, 2, -2
          j = n - k
          kk = kk + ks
          wkr = 0.5d0 - c(kk)
          wki = c(nc - kk)
          xr = a(k) - a(j)
          xi = a(k + 1) + a(j + 1)
          yr = wkr * xr - wki * xi
          yi = wkr * xi + wki * xr
          a(k) = a(k) - yr
          a(k + 1) = a(k + 1) - yi
          a(j) = a(j) + yr
          a(j + 1) = a(j + 1) - yi
      end do
      end
!
      subroutine dctsub(n, a, nc, c)
      integer n, nc, j, k, kk, ks, m
      real*8 a(0 : n - 1), c(0 : nc - 1), wkr, wki, xr
      ks = nc / n
      kk = ks
      m = n / 2
      do k = 1, m - 1
          j = n - k
          wkr = c(kk) - c(nc - kk)
          wki = c(kk) + c(nc - kk)
          kk = kk + ks
          xr = wki * a(k) - wkr * a(j)
          a(k) = wkr * a(k) + wki * a(j)
          a(j) = xr
      end do
      a(m) = 2 * c(kk) * a(m)
      end
!
      subroutine dstsub(n, a, nc, c)
      integer n, nc, j, k, kk, ks, m
      real*8 a(0 : n - 1), c(0 : nc - 1), wkr, wki, xr
      ks = nc / n
      kk = ks
      m = n / 2
      do k = 1, m - 1
          j = n - k
          wkr = c(kk) - c(nc - kk)
          wki = c(kk) + c(nc - kk)
          kk = kk + ks
          xr = wki * a(j) - wkr * a(k)
          a(j) = wkr * a(j) + wki * a(k)
          a(k) = xr
      end do
      a(m) = 2 * c(kk) * a(m)
      end
!
