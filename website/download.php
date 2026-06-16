<?php
$sourcePath = __DIR__ . '/../pew.C';
$source = null;
if (file_exists($sourcePath)) {
    $source = file_get_contents($sourcePath);
}
$download = isset($_GET['download']);
if ($download && $source) {
    header('Content-Type: text/plain; charset=utf-8');
    header('Content-Disposition: attachment; filename="pew.C"');
    echo $source;
    exit;
}
$buildInstructions = <<<HTML
<pre>gcc -x c "pew.C" -o pew -lm
./pew

# Install to ~/.local/bin
install -Dm755 pew ~/.local/bin/pew
</pre>
HTML;
?>
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Download pew | pew language</title>
  <link rel="stylesheet" href="style.css">
</head>
<body>
  <div class="download-wrapper">
    <div class="download-panel">
      <div class="download-hero">
        <h1>Download pew</h1>
        <p>Get the latest `pew.C` source from this repository, preview it, and build it locally.</p>
      </div>
      <div class="download-actions">
        <a class="btn btn-primary" href="?download=1">Download pew.C</a>
        <a class="btn btn-secondary" href="../README.md">Read docs</a>
      </div>
    </div>
    <div class="download-info">
      <section>
        <h2>Build & run</h2>
        <?php echo $buildInstructions; ?>
      </section>
      <section>
        <h2>Current source preview</h2>
        <div class="source-box">
            <pre><?php echo htmlspecialchars($source ?? 'pew.C not found in repository root'); ?></pre>
        </div>
      </section>
    </div>
  </div>
</body>
</html>
