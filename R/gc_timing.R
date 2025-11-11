#!/usr/bin/env Rscript
# Detecting likely GC events by timing spikes in R

args <- commandArgs(trailingOnly = TRUE)
iterations <- if (length(args) >= 1) as.integer(args[[1]]) else 300L
if (is.na(iterations) || iterations <= 0L) iterations <- 300L

# Work function: allocates and discards lots of memory
alloc_burst <- function(n = 20000L, size = 100L) {
  # n vectors of 'size' doubles; then drop references
  x <- vector("list", n)
  for (i in seq_len(n)) x[[i]] <- runif(size)
  # collapse to reduce chance of compiler eliding
  sum(vapply(x, sum, numeric(1)))
}

measure <- function(iters) {
  times <- numeric(iters)
  for (i in seq_len(iters)) {
    t0 <- proc.time()[[3]]
    invisible(alloc_burst())
    t1 <- proc.time()[[3]]
    times[i] <- t1 - t0
  }
  times
}

flag_spikes <- function(times, k = 3) {
  med <- median(times)
  mad <- stats::mad(times, constant = 1.4826)
  thr <- med + k * mad
  list(threshold = thr, spikes = which(times > thr))
}

main <- function() {
  cat(sprintf("Running %d iterations...\n", iterations))
  # warm up
  invisible(alloc_burst())
  times <- measure(iterations)
  spikes <- flag_spikes(times)

  cat(sprintf("Median time: %.6f s\n", median(times)))
  cat(sprintf("MAD-scaled threshold: %.6f s\n", spikes$threshold))
  cat(sprintf("Detected %d suspected GC spikes at iterations: %s\n",
              length(spikes$spikes),
              if (length(spikes$spikes)) paste(spikes$spikes, collapse = ", ") else "<none>"))

  # Print a small table of the first 10 iterations
  cat("\nFirst 10 iteration timings (s):\n")
  to_show <- head(times, 10)
  for (i in seq_along(to_show)) {
    cat(sprintf("%3d: %.6f\n", i, to_show[i]))
  }
}

if (identical(environment(), globalenv())) main()
