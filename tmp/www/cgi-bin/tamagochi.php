<?php
ini_set('display_errors', 1);
error_reporting(E_ALL);
session_start();

/* ---------- 1) TRAITEMENT POST ---------- */
$flash = '';
$cookie_flavour = $_COOKIE['flavour'] ?? null;

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['feed'])) {
    /* rassasié */
    $_SESSION['hunger'] = 0;

    /* nouveau parfum */
    $flavours     = ['choco-chip','speculoos','matcha','double-choco','framboise'];
    $new_flavour  = $flavours[array_rand($flavours)];
    setcookie('flavour', $new_flavour, time() + 3600, '/', '', false, true);

    $cookie_flavour = $new_flavour;          // ► mis à jour immédiatement
    $flash = 'Merci ! Le monstre est rassasié.';
}

/* ---------- 2) PREMIER COOKIE ---------- */
if ($cookie_flavour === null) {
    $starter        = ['vanilla','citron','dulce-de-leche','coco-nibs'];
    $cookie_flavour = $starter[array_rand($starter)];
    setcookie('flavour', $cookie_flavour, time() + 3600, '/', '', false, true);
}

/* ---------- 3) HUNGER ---------- */
$_SESSION['hunger'] = ($_SESSION['hunger'] ?? 0) + 1;
$levels = [
    ['😋','Repus et heureux !'],
    ['🙂','Ça va, encore un cookie ?'],
    ['😐','Commence à avoir faim…'],
    ['🥺','Faim… donne-moi un cookie !'],
    ['😡','TRES FAIM !!!'],
];
list($face, $speech) = $levels[min($_SESSION['hunger'], 4)];

header('Content-Type: text/html; charset=utf-8');
?>
<!DOCTYPE html><html lang="fr"><head>
<meta charset="utf-8">
<title>Cookie-Tamagochi 🍪</title>
<style>
body{font-family:system-ui,sans-serif;text-align:center;margin-top:3rem}
button{padding:.5rem 1rem;font-size:1rem;border-radius:9999px;cursor:pointer}
.notice{color:green;font-weight:600;margin:1rem 0}
</style>
</head><body>
  <h1>🍪 Cookie-Tamagochi</h1>

  <?php if($flash) echo "<p class='notice'>$flash</p>"; ?>

  <p style="font-size:5rem;line-height:1"><?= $face ?></p>
  <p><strong><?= htmlspecialchars($speech) ?></strong></p>

  <p>Parfum de ton cookie : <code><?= htmlspecialchars($cookie_flavour) ?></code></p>
  <p>Niveau de faim : <code><?= $_SESSION['hunger'] ?></code></p>

  <form method="post">
      <button name="feed" value="1">Donner un cookie 🍪</button>
  </form>

  <script>
  /* évite le « resubmit form » sur F5 */
  if (window.history.replaceState) {
    window.history.replaceState(null, '', window.location.pathname);
  }
  </script>
</body></html>
