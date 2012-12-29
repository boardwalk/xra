
(defvar xra-keywords
  (regexp-opt '("fn" "if" "else" "elsif" "while" "break" "return" "extern") 'words)
  "xra keywords")

(defvar xra-types
  (regexp-opt '("bool" "int" "float" "str") 'words)
  "xra types")

(defvar xra-constants
  (mapconcat 'identity
             '("\\<true\\>"
               "\\<false\\>"
               "0b[01]+"
               "0[0-7]+"
               "[0-9]+\\(\\.[0-9]+\\)?"
               "0x[0-9a-fA-F]+") "\\|")
  "xra constants")

(defvar xra-variables
  "\\<[a-zA-Z][a-zA-Z0-9]*\\>"
  "xra variables")

(setq xra-font-lock-keywords `(
  (,xra-keywords . font-lock-keyword-face)
  (,xra-types . font-lock-type-face)
  (,xra-constants . font-lock-constant-face)
  (,xra-variables . font-lock-variable-name-face)
))

(defvar xra-syntax-table nil)
(setq xra-syntax-table
      (let ((synTable (make-syntax-table)))
        (modify-syntax-entry ?# "< 14" synTable)
        (modify-syntax-entry ?\n ">" synTable)
        (modify-syntax-entry ?| ". 23bn" synTable)
        synTable))

(define-derived-mode xra-mode fundamental-mode
  "xra mode"
  "Major mode for editing xra"
  :syntax-table xra-syntax-table
  (setq font-lock-defaults '((xra-font-lock-keywords))))

(provide 'xra-mode)
