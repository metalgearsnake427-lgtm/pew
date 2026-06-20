use crate::git::Commit;

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Emotion {
    Ambition,   // new features, init, add
    Struggle,   // fix, bug, broken, why
    Triumph,    // finally, done, complete, works
    Panic,      // hotfix, revert, critical, urgent
    Cleanup,    // refactor, remove, clean, reorganize
    Despair,    // wtf, again, ugh, sigh, still, another
    Mystery,    // no message / single word
}

impl Emotion {
    pub fn char(&self) -> &'static str {
        match self {
            Emotion::Ambition => "▲",
            Emotion::Struggle => "~",
            Emotion::Triumph  => "★",
            Emotion::Panic    => "!",
            Emotion::Cleanup  => "◈",
            Emotion::Despair  => "▼",
            Emotion::Mystery  => "?",
        }
    }

    pub fn label(&self) -> &'static str {
        match self {
            Emotion::Ambition => "Ambition",
            Emotion::Struggle => "Struggle",
            Emotion::Triumph  => "Triumph",
            Emotion::Panic    => "Panic",
            Emotion::Cleanup  => "Cleanup",
            Emotion::Despair  => "Despair",
            Emotion::Mystery  => "Mystery",
        }
    }

    pub fn score(&self) -> f32 {
        match self {
            Emotion::Ambition => 0.7,
            Emotion::Struggle => 0.3,
            Emotion::Triumph  => 1.0,
            Emotion::Panic    => 0.1,
            Emotion::Cleanup  => 0.6,
            Emotion::Despair  => 0.0,
            Emotion::Mystery  => 0.5,
        }
    }
}

const PANIC_WORDS:   &[&str] = &["hotfix","urgent","critical","revert","emergency","broke","disaster","rollback","incident"];
const DESPAIR_WORDS: &[&str] = &["wtf","again","still","another","why","ugh","sigh","mess","terrible","awful","hate","broke again","not again","same","always"];
const TRIUMPH_WORDS: &[&str] = &["finally","done","complete","finished","working","works","ships","solved","fixed it","perfect","!!","nailed","success","yay","it works"];
const STRUGGLE_WORDS:&[&str] = &["fix","bug","patch","issue","error","broken","doesnt","doesn't","fail","crash","wrong","bad","oops","mistake","off by","typo"];
const AMBITION_WORDS:&[&str] = &["add","new","implement","feature","initial","start","begin","create","introduce","support","enable","allow","build","design","draft"];
const CLEANUP_WORDS: &[&str] = &["refactor","clean","remove","delete","simplify","reorganize","rename","move","extract","improve","tidy","format","lint","style","dead code"];

pub fn detect(commit: &Commit) -> Emotion {
    let msg = commit.subject.to_lowercase();

    // apply time-of-day modifier: 3am pushes toward despair/panic
    let is_late = commit.hour >= 23 || commit.hour < 4;

    // score each bucket
    let panic   = score(&msg, PANIC_WORDS);
    let despair = score(&msg, DESPAIR_WORDS);
    let triumph = score(&msg, TRIUMPH_WORDS);
    let struggle= score(&msg, STRUGGLE_WORDS);
    let ambition= score(&msg, AMBITION_WORDS);
    let cleanup = score(&msg, CLEANUP_WORDS);

    // short / empty messages are mystery
    if msg.trim().len() <= 3 || msg == "wip" || msg == "." || msg == "update" || msg == "changes" {
        return Emotion::Mystery;
    }

    let mut best = Emotion::Ambition;
    let mut best_score = 0usize;

    for (score, emotion) in [
        (panic,   Emotion::Panic),
        (despair, Emotion::Despair),
        (triumph, Emotion::Triumph),
        (struggle,Emotion::Struggle),
        (ambition,Emotion::Ambition),
        (cleanup, Emotion::Cleanup),
    ] {
        if score > best_score {
            best_score = score;
            best = emotion;
        }
    }

    // late-night override: struggle → despair, ambition → struggle
    if is_late {
        best = match best {
            Emotion::Struggle => Emotion::Despair,
            Emotion::Ambition => Emotion::Struggle,
            other             => other,
        };
    }

    best
}

fn score(msg: &str, words: &[&str]) -> usize {
    words.iter().filter(|&&w| msg.contains(w)).count()
}

/// Rolling average emotion score over a window of commits
pub fn rolling_mood(emotions: &[Emotion], window: usize) -> Vec<f32> {
    emotions
        .windows(window)
        .map(|w| w.iter().map(|e| e.score()).sum::<f32>() / w.len() as f32)
        .collect()
}
