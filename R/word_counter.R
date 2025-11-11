#!/usr/bin/env Rscript
# Word frequency counter in R (case-insensitive, ignore punctuation)
# Usage: Rscript R/word_counter.R <filename>

script_name <- (function(default = "word_counter.R") {
  args <- commandArgs(trailingOnly = FALSE)
  file_arg <- grep("^--file=", args, value = TRUE)
  if (length(file_arg) > 0) {
    return(basename(sub("^--file=", "", tail(file_arg, n = 1))))
  }
  default
})()

args <- commandArgs(trailingOnly = TRUE)
if (length(args) != 1) {
  write(sprintf("Usage: Rscript %s <file>", script_name), stderr())
  quit(status = 1)
}

fn <- args[[1]]

safe_read_lines <- function(path) {
  tryCatch(readLines(path, warn = FALSE, encoding = "UTF-8"),
           error = function(e) {
             write(sprintf("Failed to open '%s': %s", path, conditionMessage(e)), stderr())
             quit(status = 1)
           })
}

lines <- safe_read_lines(fn)
text <- tolower(paste(lines, collapse = " "))
# Replace any non-alphanumeric with space
text <- gsub("[^a-z0-9]+", " ", text, perl = TRUE)
# Split on whitespace and drop empties
words <- unlist(strsplit(text, "\\s+", perl = TRUE), use.names = FALSE)
words <- words[nzchar(words)]

if (length(words) == 0) {
  cat("No words found.\n")
  quit(status = 0)
}

counts <- sort(table(words), decreasing = TRUE)
# Top 20
n <- min(20L, length(counts))
cat("Top 20 words by frequency:\n")
for (i in seq_len(n)) {
  w <- names(counts)[i]
  c <- as.integer(counts[[i]])
  # pad word column to ~10 chars to mimic C output
  cat(sprintf("%-10s\t%d\n", w, c))
}
