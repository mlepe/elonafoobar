locale {
    food {
        # Names for cooked food.
        # These are organized by type, then quality.
        names {
            # Meat
            _1 {
                default_origin = "動物"
                _1 = "グロテスクな${_1}の肉"
                _2 = "焼け焦げた${_1}の肉"
                _3 = "${_1}のこんがり肉"
                _4 = "${_1}肉のオードブル"
                _5 = "${_1}のピリ辛炒め"
                _6 = "${_1}コロッケ"
                _7 = "${_1}のハンバーグ"
                _8 = "${_1}肉の大葉焼き"
                _9 = "${_1}ステーキ"
            }
            # Vegetable
            _2 {
                default_origin = "野菜"
                _1 = "生ごみ同然の${_1}"
                _2 = "悪臭を放つ${_1}"
                _3 = "${_1}のサラダ"
                _4 = "${_1}の炒め物"
                _5 = "${_1}風味の肉じゃが"
                _6 = "${_1}の天ぷら"
                _7 = "${_1}の煮込み"
                _8 = "${_1}シチュー"
                _9 = "${_1}風カレー"
            }
            # Fruit
            _3 {
                default_origin = "果物"
                _1 = "食べてはならない${_1}"
                _2 = "べっちょりした${_1}"
                _3 = "${_1}のフルーツサラダ"
                _4 = "${_1}のプリン"
                _5 = "${_1}シャーベット"
                _6 = "${_1}シェイク"
                _7 = "${_1}クレープ"
                _8 = "${_1}フルーツケーキ"
                _9 = "${_1}パフェ"
            }
            # Candy
            _4 {
                default_origin = "お菓子"
                _1 = "原型を留めない${_1}"
                _2 = "まずそうな${_1}"
                _3 = "${_1}クッキー"
                _4 = "${_1}のゼリー"
                _5 = "${_1}パイ"
                _6 = "${_1}まんじゅう"
                _7 = "${_1}風味のシュークリーム"
                _8 = "${_1}のケーキ"
                _9 = "${_1}風ザッハトルテ"
            }
            # Noodle
            _5 {
                default_origin = "麺"
                _1 = "禁断の${_1}"
                _2 = "のびてふにゃった${_1}"
                _3 = "サラダパスタ"
                _4 = "うどん"
                _5 = "冷やし蕎麦"
                _6 = "ペペロンチーノ"
                _7 = "カルボナーラ"
                _8 = "ラーメン"
                _9 = "ミートスパゲティ"
            }
            # Fish
            _6 {
                default_origin = "魚"
                _1 = "${_1}の残骸"
                _2 = "骨だけ残った${_1}"
                _3 = "${_1}のフライ"
                _4 = "${_1}の煮込み"
                _5 = "${_1}スープ"
                _6 = "${_1}の天ぷら"
                _7 = "${_1}ソーセージ"
                _8 = "${_1}の刺身"
                _9 = "${_1}の活け作り"
            }
            # Bread
            _7 {
                default_origin = "パン"
                _1 = "恐怖の${_1}"
                _2 = "ガチガチの${_1}"
                _3 = "くるみパン"
                _4 = "アップルパイ"
                _5 = "サンドイッチ"
                _6 = "クロワッサン"
                _7 = "コロッケパン"
                _8 = "カレーパン"
                _9 = "メロンパン"
            }
            # Egg
            _8 {
                default_origin = "鳥"
                _1 = "グロテスクな${_1}の卵"
                _2 = "焦げた${_1}の卵"
                _3 = "${_1}の卵の目玉焼き"
                _4 = "${_1}風味のキッシュ"
                _5 = "半熟${_1}"
                _6 = "${_1}の卵入りスープ"
                _7 = "熟成${_1}チーズ"
                _8 = "${_1}のレアチーズケーキ"
                _9 = "${_1}風オムライス"
            }
        }

        passed_rotten {
            _0 = "「うぐぐ！なんだこの飯は！」"
            _1 = "「うっ！」"
            _2 = "「……！！」"
            _3 = "「あれれ…」"
            _4 = "「…これは何の嫌がらせですか」"
            _5 = "「まずい！」"
        }

        mochi {
            chokes = "${name(_1)}はもちを喉につまらせた！"
            dialog = "「むがっ」"
        }

        hunger_status {
            hungry {
                _0 = "腹がすいてきた。"
                _1 = "空腹になった。"
                _2 = "さて何を食べようか。"
            }
            very_hungry {
                _0 = "空腹で目が回りだした…"
                _1 = "すぐに何かを食べなくては…"
            }
            starving {
                _0 = "このままだと餓死してしまう！"
                _1 = "腹が減ってほとんど死にかけている。"
            }
        }

        eating_message {
            bloated {
                _0 = "もう当分食べなくてもいい。"
                _1 = "こんなに食べたことはない！"
                _2 = "信じられないぐらい満腹だ！"
            }
            satisfied {
                _0 = "あなたは満足した。"
                _1 = "満腹だ！"
                _2 = "あなたは食欲を満たした。"
                _3 = "あなたは幸せそうに腹をさすった。"
            }
            normal {
                _0 = "まだ食べられるな…"
                _1 = "あなたは腹をさすった。"
                _2 = "少し食欲を満たした。"
            }
            hungry {
                _0 = "まだまだ食べたりない。"
                _1 = "物足りない…"
                _2 = "まだ空腹だ。"
                _3 = "少しは腹の足しになったか…"
            }
            very_hungry {
                _0 = "全然食べたりない！"
                _1 = "腹の足しにもならない。"
                _2 = "すぐにまた腹が鳴った。"
            }
            starving {
                _0 = "こんな量では意味がない！"
                _1 = "これぐらいでは、死を少し先に延ばしただけだ。"
                _2 = "無意味だ…もっと栄養をとらなければ。"
            }
        }

        not_affected_by_rotten = "しかし、${name(_1)}は何ともなかった。"

        anorexia {
            develops = "${name(_1)}は拒食症になった。"
            recovers_from = "${name(_1)}の拒食症は治った。"
        }

        vomits = "${name(_1)}は吐いた。"
        spits_alien_children = "${name(_1)}は体内のエイリアンを吐き出した！"

        eat_status {
            good = "${name(_1)}は良い予感がした。"
            bad = "${name(_1)}は嫌な感じがした。"
            cursed_drink = "${name(_1)}は気分が悪くなった。"
        }

        cook = "${itemname(_2, 1)}で${_1}を料理して、${itemname(_3, 1)}を作った。"

        effect {
            rotten = "うげっ！腐ったものを食べてしまった…うわ…"

            raw_meat = "生肉だ…"
            powder = "粉の味がする…"
            raw = "生で食べるものじゃないな…"
            boring {
                _0 = "まずいわけではないが…"
                _1 = "平凡な味だ。"
            }

            raw_glum = "${name(_1)}は渋い顔をした。"

            herb {
                curaria = "このハーブは活力の源だ。"
                morgia = "新たな力が湧きあがってくる。"
                mareilon = "魔力の向上を感じる。"
                spenseweed = "感覚が研ぎ澄まされるようだ。"
                alraunia = "ホルモンが活発化した。"
            }

            human {
                like = "これはあなたの大好きな人肉だ！"
                dislike = "これは人肉だ…うぇぇ！"
                would_have_rather_eaten = "人肉の方が好みだが…"
                delicious = "ウマイ！"
            }

            bomb_fish = "「げふぅ」${name(_1)}は${itemname(_2, 1)}を吐き出した。"
            little_sister = "${name(_1)}は進化した。"
            hero_cheese = "これは縁起がいい！"

            fortune_cookie = "${name(_1)}はクッキーの中のおみくじを読んだ。"
            sisters_love_fueled_lunch = "${name(_1)}の心はすこし癒された。"

            poisoned {
                text = "これは毒されている！${name(_1)}はもがき苦しみのたうちまわった！"
                dialog {
                    _0 = "「ギャァァ…！」"
                    _1 = "「ブッ！」"
                }
            }

            spiked {
                self = "あなたは興奮した！"
                other {
                    _0 = "${name(_1)}「なんだか…変な気分なの…」"
                    _1 = "${name(_1)}「あれ…なにこの感じは…」"
                }
            }

            ability {
                develops = "${name(_1)}の${_2}は発達した。"
                deteriorates = "${name(_1)}の${_2}は衰えた。"
            }

            growth = "${name(_1)}の${_2}は成長期に突入した。"

            corpse {
                iron = "まるで鉄のように硬い！${name(_1)}の胃は悲鳴をあげた。"
                deformed_eye = "気が変になりそうな味だ。"
                horse = "馬肉だ！これは精がつきそうだ。"
                holy_one = "${name(_1)}は神聖なものを汚した気がした。"
                at = "＠を食べるなんて…"
                guard = "ガード達はあなたを憎悪した。"
                vesda = "${name(_1)}の体は一瞬燃え上がった。"
                insanity = "${name(_1)}の胃は狂気で満たされた。"
                putit = "肌がつるつるになりそうだ。"
                cupid_of_love = "${name(_1)}は恋をしている気分になった！"
                poisonous = "これは有毒だ！"
                troll = "血が沸き立つようだ。"
                rotten_one = "腐ってるなんて分かりきっていたのに…うげぇ"
                beetle = "力が湧いてくるようだ。"
                mandrake = "微かな魔力の刺激を感じた。"
                grudge = "胃の調子がおかしい…"
                calm = "この肉は心を落ち着かせる効果があるようだ。"
                chaos_cloud = "${name(_1)}の胃は混沌で満たされた。"
                lightning = "${name(_1)}の神経に電流が走った。"
                cat = "猫を食べるなんて！！"
                ether = "${name(_1)}の体内はエーテルで満たされた。"
                giant = "体力がつきそうだ。"
                imp = "魔力が鍛えられる。"
                strength = "力がつきそうだ。"
                ghost = "精神が少しずぶとくなった。"
                quickling = "ワアーォ、${name(_1)}は速くなった気がする！"
                alien = "何かが${name(_1)}の体内に入り込んだ。"
            }

            quality {
                bad {
                    _0 = "うぅ…腹を壊しそうだ。"
                    _1 = "まずい！"
                    _2 = "ひどい味だ！"
                }
                so_so {
                    _0 = "まあまあの味だ。"
                    _1 = "悪くない味だ。"
                }
                good {
                    _0 = "かなりいける。"
                    _1 = "それなりに美味しかった。"
                }
                great {
                    _0 = "美味しい！"
                    _1 = "これはいける！"
                    _2 = "いい味だ！"
                }
                delicious {
                    _0 = "最高に美味しい！"
                    _1 = "まさに絶品だ！"
                    _2 = "天にも昇る味だ！"
                }
            }
        }
    }
}