use crate::emotion::{rolling_mood, Emotion};
use crate::git::RepoStats;
use crate::narrative::{CharacterProfile, Story};
use colored::*;

pub fn print_banner() {
    println!(
        "{}",
        r#"
   __ _  _  _     _     _                        
  / _` || || |   | |__ | |  __ _  _ __ ___    ___
 | (_| || || |_  | '_ \| | / _` || '_ ` _ \  / _ \
  \__, ||_||_(_) | |_) | || (_| || | | | | ||  __/
  |___/           |_.__/|_| \__,_||_| |_| |_| \___|
  ──  h u m a n  ──────────────────────────────────"#
            .bright_purple()
    );
    println!("{}", "  No AI. No API key. Just your git history.\n".dimmed());
}

fn sep(n: usize) {
    println!("{}", "─".repeat(n).bright_purple());
}

pub fn print_stats(stats: &RepoStats) {
    sep(60);
    println!("  {}", stats.name.bold().bright_white());
    sep(60);

    let days = ((stats.last_ts - stats.first_ts) / 86400).max(1);
    println!("  {}  {}", "commits  :".bright_cyan(), stats.total_commits.to_string().bright_white());
    println!("  {}  {}", "authors  :".bright_cyan(), stats.authors.len().to_string().bright_white());
    println!("  {}  {} days", "lifespan :".bright_cyan(), days.to_string().bright_white());

    if !stats.authors.is_empty() {
        println!("\n  {}", "contributors".bright_cyan().bold());
        let max = stats.authors[0].commits;
        for a in stats.authors.iter().take(5) {
            let bar_len = (a.commits * 24 / max.max(1)).max(1);
            let bar = "█".repeat(bar_len) + &"░".repeat(24 - bar_len);
            println!("    {} {}  {}",
                bar.dimmed(),
                a.name.bright_yellow(),
                a.commits.to_string().dimmed()
            );
        }
    }

    if !stats.hottest_files.is_empty() {
        println!("\n  {}", "most rewritten files".bright_cyan().bold());
        for (f, n) in &stats.hottest_files {
            println!("    {} {} {}", "●".bright_red(), f.bright_white(), format!("({}×)", n).dimmed());
        }
    }

    sep(60);
    println!();
}

// ─── Emotion timeline graph ───────────────────────────────────────────────────

pub fn print_emotion_graph(emotions: &[Emotion]) {
    if emotions.len() < 5 { return; }

    let window = (emotions.len() / 20).max(3);
    let mood   = rolling_mood(emotions, window);

    let height = 6usize;
    let width  = mood.len().min(56);
    let step   = (mood.len() as f32 / width as f32).max(1.0);

    println!("  {}", "emotional arc of this project".bright_cyan().bold());
    println!("  {}", "(rolling mood — ▲ triumph  ▼ despair  ! panic)".dimmed());
    println!();

    // build grid: height rows × width cols
    let sampled: Vec<f32> = (0..width)
        .map(|i| mood[(i as f32 * step) as usize])
        .collect();

    for row in (0..height).rev() {
        let threshold = row as f32 / (height - 1) as f32;
        let mut line  = String::from("  ");
        for &v in &sampled {
            let ch = if v >= threshold { "█" } else { " " };
            let colored = if v >= 0.8 { ch.bright_green() }
                     else if v >= 0.5 { ch.bright_cyan()  }
                     else if v >= 0.3 { ch.bright_yellow() }
                     else             { ch.bright_red()   };
            line.push_str(&colored.to_string());
        }
        if row == height - 1 { line.push_str(&"  triumph".bright_green().to_string()); }
        if row == 0          { line.push_str(&"  despair".bright_red().to_string());   }
        println!("{}", line);
    }

    // x-axis: Early / Mid / Late
    let pad = width / 3;
    println!("  {}{}{}",
        "Early".dimmed(),
        " ".repeat(pad.saturating_sub(3)),
        "Recent".dimmed()
    );
    println!();

    // emotion legend with real counts
    let counts: Vec<(&str, usize, &str)> = vec![
        (Emotion::Ambition.char(), emotions.iter().filter(|&&e| e == Emotion::Ambition).count(), Emotion::Ambition.label()),
        (Emotion::Triumph.char(),  emotions.iter().filter(|&&e| e == Emotion::Triumph).count(),  Emotion::Triumph.label()),
        (Emotion::Struggle.char(), emotions.iter().filter(|&&e| e == Emotion::Struggle).count(), Emotion::Struggle.label()),
        (Emotion::Panic.char(),    emotions.iter().filter(|&&e| e == Emotion::Panic).count(),    Emotion::Panic.label()),
        (Emotion::Despair.char(),  emotions.iter().filter(|&&e| e == Emotion::Despair).count(),  Emotion::Despair.label()),
        (Emotion::Cleanup.char(),  emotions.iter().filter(|&&e| e == Emotion::Cleanup).count(),  Emotion::Cleanup.label()),
        (Emotion::Mystery.char(),  emotions.iter().filter(|&&e| e == Emotion::Mystery).count(),  Emotion::Mystery.label()),
    ];

    print!("  ");
    for (ch, count, label) in &counts {
        if *count > 0 {
            print!("{}{} {}   ", ch.bright_white(), ch.dimmed(), format!("{} {}", count, label).dimmed());
        }
    }
    println!("\n");
}

// ─── story ───────────────────────────────────────────────────────────────────

pub fn print_story(story: &Story) {
    println!("{}", format!("  ◆  {}", story.title).bright_white().bold());
    println!("{}", "  ─────────────────────────────────────────────────────".bright_purple());
    println!();

    for chapter in &story.chapters {
        println!("  {}", chapter.heading.bright_cyan().bold());
        println!();
        for line in wrap(&chapter.body, 66) {
            println!("  {}", line.bright_white());
        }
        println!();
    }
}

// ─── character profiles ───────────────────────────────────────────────────────

pub fn print_characters(characters: &[CharacterProfile]) {
    println!("{}", "  ─────────────────────────────────────────────────────".bright_purple());
    println!("  {}", "the people behind the commits".bright_cyan().bold());
    println!();

    for c in characters {
        println!("  {} {}  {}",
            c.badge,
            c.name.bright_yellow().bold(),
            c.archetype.dimmed()
        );
        for line in wrap(&c.summary, 64) {
            println!("    {}", line.bright_white());
        }
        println!();
    }
}

// ─── verdict ─────────────────────────────────────────────────────────────────

pub fn print_verdict(verdict: &str) {
    println!("{}", "  ─────────────────────────────────────────────────────".bright_purple());
    println!("  {}", "verdict".bright_cyan().bold());
    println!();
    for line in wrap(verdict, 66) {
        println!("  {}", line.dimmed());
    }
    println!();
    println!("{}", "  ─────────────────────────────────────────────────────".bright_purple());
}

// ─── word wrap (stdlib only) ──────────────────────────────────────────────────

fn wrap(text: &str, width: usize) -> Vec<String> {
    let mut lines = Vec::new();
    let mut cur   = String::new();
    for word in text.split_whitespace() {
        if cur.is_empty() {
            cur.push_str(word);
        } else if cur.len() + 1 + word.len() <= width {
            cur.push(' ');
            cur.push_str(word);
        } else {
            lines.push(cur.clone());
            cur = word.to_string();
        }
    }
    if !cur.is_empty() { lines.push(cur); }
    lines
}
