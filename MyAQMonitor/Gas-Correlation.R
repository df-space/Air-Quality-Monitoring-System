# An R script to estimate MQ gas sensors correlation curve and compute Ro, min and max Rs/Ro
#
# Copyright (c) Davide Gironi, 2016
#
# Released under GPLv3.
# Please refer to LICENSE file for licensing information.

# How to use this script:
# 1) set limits as datasheet curve ("xlim" and "ylim")
#    ex.
#      xlim = c(10, 1000)
#      ylim = c(0.1, 10)
# 2) find out datasheet curve points, and write it out (to "pointsdata")
#    each line it's a point on cartesian coordinate system
#    the useful WebPlotDigitizer app can help you extract points from the graph
#    ex.
#      pointsdata = "
#        10.052112405371744, 2.283698378106183
#        20.171602728600178, 1.8052797165878915
#        30.099224396434586, 1.5715748803154423
#        50.09267987761949, 1.3195287228519417
#        80.38812026903305, 1.1281218760133969
#        90.12665922665023, 1.0815121769656304
#        100.52112405371739, 1.0430967861855598
#        199.62996638292853, 0.8000946404902397
#      "
# 3) optional for Ro estimation: measure the sensor resistance (set it to "mres" ohm value) at a know amount of gas
#    set it to 0 if you do not need the Ro estimation
#    ex.
#      mres = 26954
# 4) optional for Ro estimation: set the know amount of gas for the resistance measure of the previous step (to "mppm")
#    set it to 0 if you do not need the Ro estimation
#    ex.
#      mppm = 392
# 5) optional for min-max Rs/Ro estimation: set the minand max amount of gas the sensor will react to (as "minppm" and "maxppm")
#    set it to 0 if you do not need the min-max Rs/Ro estimation
#    ex.
#      minppm = 10
#      maxppm = 200
library(data.table)

#remove old variables
rm(list=ls())

#set input values
xlim = c(10, 1000)
ylim = c(0.1, 10)
minppm = 0
maxppm = 0
mres = 0
mppm = 0
pointsdata = "
9.883667867092264, 2.5603804253891482
17.856006318670133, 2.0321627084619878
19.933413523351835, 1.9219276029960917
29.70394486082898, 1.6519354746836543
39.38035774267106, 1.4658514650118017
50.098463608764156, 1.3216220214282914
59.08841626703225, 1.2499303491465668
69.21584841825364, 1.1634398754976438
79.42243366916493, 1.1003288279202683
88.05319774825183, 1.0573565337706017
100.34306738301912, 1.0160624816315302
132.12291723271093, 0.8873539478244531
171.582489168588, 0.8064443331014424
196.87471838367696, 0.7873970287196413"

#load points using fread
setnames(points <- fread(pointsdata, sep=",", sep2="\n"), c("x","y"))

#set named list of points, and swapped list of points
#points will be used to plot and compute values as datasheet figure
#pointsrev will be used to plot and compute values for the correlation function, it's the datasheet figure with swapped axis
x <- as.vector(points[,x])
y <- as.vector(points[,y])
points = list(x=x, y=y)
pointsrev = list(x=y, y=x)

#the nls (Nonlinear Least Squares) it's used to perform the power regression on points
#in order to work, nls needs an estimation of staring values
#we use log-log slope estimation to find intitial values

#estimate fit curve initial values
xfirst = head(points$x, n=1)
xlast = tail(points$x, n=1)
yfirst = head(points$y, n=1)
ylast = tail(points$y, n=1)
bstart= log(ylast/yfirst)/log(xlast/xfirst)
astart = yfirst/(xfirst^bstart)
#perform the fit
fit <- nls("y~a*x^b", start=list(a=astart,b=bstart), data=points)

#estimate fitref curve initial values
xfirstrev = head(pointsrev$x, n=1)
xlastrev = tail(pointsrev$x, n=1)
yfirstrev = head(pointsrev$y, n=1)
ylastrev = tail(pointsrev$y, n=1)
bstartrev = log(ylastrev/yfirstrev)/log(xlastrev/xfirstrev)
astartrev = yfirstrev/(xfirstrev^bstartrev)
fitrev <- nls("y~a*x^b", start=list(a=astartrev,b=bstartrev), data=pointsrev)

#plot fit curve (log-log scale)
fiteq = function(x){coef(fit)["a"]*x^(coef(fit)["b"])}
plot(points, log="xy", col="blue", xlab="ppm", ylab="Rs/Ro", xlim=xlim, ylim=ylim, panel.first=grid(equilogs=FALSE))
curve(fiteq, col="red", add=TRUE)

#plot fitrev curve (log-log scale)
fiteqrev = function(x){coef(fitrev)["a"]*x^(coef(fitrev)["b"])}
plot(pointsrev, log="xy", col="blue", xlab="Rs/Ro", ylab="ppm", xlim=ylim, ylim=xlim, panel.first=grid(equilogs=FALSE))
curve(fiteqrev, col="red", add=TRUE)

#plot fit curve (linear scale)
fiteq = function(x){coef(fit)["a"]*x^(coef(fit)["b"])}
plot(points, col="blue", xlab="ppm", ylab="Rs/Ro", panel.first=grid(equilogs=FALSE))
curve(fiteq, col="red", add=TRUE)

#plot fitrev curve (linear scale)
fiteqrev = function(x){coef(fitrev)["a"]*x^(coef(fitrev)["b"])}
plot(pointsrev, col="blue", xlab="Rs/Ro", ylab="ppm", panel.first=grid(equilogs=FALSE))
curve(fiteqrev, col="red", add=TRUE)

#estimate min Rs/Ro
cat("\nCorrelation function coefficients")
cat("\nEstimated a\n")
cat("  ")
cat(coef(fitrev)["a"])
cat("\nEstimated b\n")
cat("  ")
cat(coef(fitrev)["b"])
cat("\n")

#estimate min Rs/Ro
if (minppm != 0) {
    minRsRo = (maxppm/coef(fitrev)["a"])^(1/coef(fitrev)["b"])
    cat("\nEstimated min Rs/Ro\n")
    cat("  ")
    cat(minRsRo)
    cat("\n")
}

#estimate max Rs/Ro
if (maxppm != 0) {
    maxRsRo = (minppm/coef(fitrev)["a"])^(1/coef(fitrev)["b"])
    cat("\nEstimated max Rs/Ro\n")
    cat("  ")
    cat(maxRsRo)
    cat("\n")
}

#estimate Ro
if (mppm != 0 && mres != 0) {
    Ro = mres*(coef(fitrev)["a"]/mppm)^(1/coef(fitrev)["b"])
    cat("\nEstimated Ro\n")
    cat("  ")
    cat(Ro)
    cat("\n")
}