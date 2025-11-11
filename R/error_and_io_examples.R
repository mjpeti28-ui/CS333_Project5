#!/usr/bin/env Rscript
# Error handling and file I/O examples in base R

# Error handling --------------------------------------------------------------

# Demonstrate tryCatch and withCallingHandlers usage.
examples_error_handling <- function() {
  cat("Example: tryCatch with specific handlers\n")
  f <- function(x) {
    if (x < 0) warning("x is negative")
    if (x == 0) stop("x cannot be zero")
    10 / x
  }

  for (x in c(2, -1, 0)) {
    res <- tryCatch(
      f(x),
      warning = function(w) {
        cat("  caught warning:", conditionMessage(w), "\n")
        NA_real_
      },
      error = function(e) {
        cat("  caught error:", conditionMessage(e), "\n")
        NA_real_
      },
      finally = {
        cat("  finally after x=", x, "\n", sep = "")
      }
    )
    cat("  result:", res, "\n")
  }

  cat("\nExample: withCallingHandlers for warnings (non-local handling)\n")
  withCallingHandlers({
    warning("this is a warning")
    cat("  continued after warning\n")
  }, warning = function(w) {
    cat("  observed warning:", conditionMessage(w), "\n")
    invokeRestart("muffleWarning")
  })
}

# Text file I/O ---------------------------------------------------------------

# Show how to write/read plain text files.
examples_text_io <- function(tmpdir = tempdir()) {
  path <- file.path(tmpdir, "sample.txt")
  cat("hello\nworld\n", file = path)

  cat("Wrote:", path, "\n")

  lines <- readLines(path, warn = FALSE)
  cat("ReadLines result:", paste(lines, collapse = ", "), "\n")

  con <- file(path, open = "r", encoding = "UTF-8")
  on.exit(close(con), add = TRUE)
  cat("First line via connection:", readLines(con, n = 1), "\n")
}

# Binary I/O ------------------------------------------------------------------

# Show binary write/read with explicit sizes and endian.
examples_binary_io <- function(tmpdir = tempdir()) {
  path <- file.path(tmpdir, "sample.bin")
  con <- file(path, open = "wb")
  writeBin(as.integer(c(1, 256, 65535)), con, size = 4, endian = "little")
  close(con)

  con <- file(path, open = "rb")
  vals <- readBin(con, what = integer(), n = 3, size = 4, endian = "little")
  close(con)
  cat("Binary roundtrip:", paste(vals, collapse = ", "), "\n")
}

# URL reading -----------------------------------------------------------------

# Fetch a tiny payload over HTTP with error handling.
examples_url_io <- function() {
  txt <- tryCatch({
    con <- url("https://httpbin.org/uuid", open = "r")
    on.exit(close(con), add = TRUE)
    readLines(con, warn = FALSE)
  }, error = function(e) {
    cat("Unable to fetch URL:", conditionMessage(e), "\n")
    NULL
  })

  if (!is.null(txt) && length(txt) > 0) {
    cat("Fetched bytes from URL (first line):", substr(txt[1], 1, 60), "...\n")
  }
}

# Interactive input -----------------------------------------------------------

# Demonstrate readline() when running interactively.
examples_interactive <- function() {
  if (!interactive()) {
    cat("Not interactive; skipping readline() demo.\n")
    return(invisible(NULL))
  }
  name <- readline(prompt = "Enter your name: ")
  cat("Hello,", name, "!\n")
}

# Main driver -----------------------------------------------------------------

if (identical(environment(), globalenv())) {
  cat("-- Error handling --\n"); examples_error_handling(); cat("\n")
  cat("-- Text I/O --\n"); examples_text_io(); cat("\n")
  cat("-- Binary I/O --\n"); examples_binary_io(); cat("\n")
  cat("-- URL I/O --\n"); examples_url_io(); cat("\n")
  cat("-- Interactive --\n"); examples_interactive(); cat("\n")
}
