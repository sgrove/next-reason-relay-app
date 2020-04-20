%raw
"require('../styles/main.css')";

module P = {
  [@react.component]
  let make = (~children) => <p className="mb-2"> children </p>;
};

// ask Sean later about GitHubMilestone. looking in module

let loginToGitHub = (~auth, ~onIsLoggedIn, ~onIsNotLoggedIn=?) => {
  OneGraphAuth.login(auth, "github")
  |> Js.Promise.then_(() => {
       OneGraphAuth.isLoggedIn(auth, "github")
       |> Js.Promise.then_(isLoggedIn => {
            (
              switch (isLoggedIn) {
              | false =>
                onIsNotLoggedIn->Belt.Option.forEach(onIsNotLoggedIn =>
                  onIsNotLoggedIn()
                )
              | true => onIsLoggedIn()
              }
            )
            |> Js.Promise.resolve
          })
     });
};

let logoutOfGitHub = (~auth, ~onIsLoggedIn=?, ~onIsNotLoggedIn) => {
  OneGraphAuth.logout(auth, "github", ())
  |> Js.Promise.then_(_ => {
       OneGraphAuth.isLoggedIn(auth, "github")
       |> Js.Promise.then_(isLoggedIn => {
            (
              switch (isLoggedIn) {
              | false => onIsNotLoggedIn()
              | true =>
                onIsLoggedIn->Belt.Option.forEach(onIsLoggedIn =>
                  onIsLoggedIn()
                )
              }
            )
            |> Js.Promise.resolve
          })
     });
};

[@react.component]
let make = () => {
  let (isLoggedIn, setIsLoggedIn) = React.useState(() => None);
  let (packageName, setPackageName) = React.useState(() => "nextjs");
  let (repo, _setRepo) =
    React.useState(() => GitHubMilestone.{name: "blog", owner: "sgrove"});

  /* Determine initial login state */
  React.useEffect0(() => {
    Config.auth->Belt.Option.forEach(auth =>
      OneGraphAuth.isLoggedIn(auth, "github")
      |> Js.Promise.then_(isLoggedIn => {
           Js.Promise.resolve(setIsLoggedIn(_ => Some(isLoggedIn)))
         })
      |> ignore
    );
    None;
  });

  <div>
    <React.Suspense
      fallback={<div> "One second, loading milestones..."->React.string </div>}>
      <h1 className="text-3xl font-semibold">
        "GitHubMilestones: "->React.string
      </h1>
      <GitHubMilestone repo />
    </React.Suspense>
    <br />
    <h1 className="text-3xl font-semibold">
      {j|npm downloads for|j}->React.string
      <input
        type_="text"
        defaultValue=packageName
        onKeyDown={event => {
          let key = ReactEvent.Keyboard.key(event);
          switch (key) {
          | "Enter" =>
            ReactEvent.Keyboard.preventDefault(event);
            ReactEvent.Keyboard.stopPropagation(event);
            let newPackageName = ReactEvent.Keyboard.target(event)##value;
            setPackageName(_ => newPackageName);
          | _ => ()
          };
        }}
      />
      {j|package:|j}->React.string
    </h1>
    <React.Suspense
      fallback={
        <div>
          {j|One second, loading npm download stats for $packageName...|j}
          ->React.string
        </div>
      }>
      <UserProfile name=packageName />
    </React.Suspense>
    <br />
    <React.Suspense
      fallback={<div> "One second, loading milestones..."->React.string </div>}>
      <h1 className="text-3xl font-semibold">
        "Live npm subscription with optional GitHub data: "->React.string
      </h1>
      <br />
      <NpmPackagesLive />
    </React.Suspense>
    <hr />
    <button
      onClick={_ => {
        switch (Config.auth, isLoggedIn) {
        | (Some(auth), Some(true)) =>
          logoutOfGitHub(
            ~auth,
            ~onIsLoggedIn=() => setIsLoggedIn(_ => Some(true)),
            ~onIsNotLoggedIn=() => setIsLoggedIn(_ => Some(false)),
          )
          ->ignore

        | (Some(auth), Some(false)) =>
          loginToGitHub(
            ~auth,
            ~onIsLoggedIn=() => setIsLoggedIn(_ => Some(true)),
            ~onIsNotLoggedIn=() => setIsLoggedIn(_ => Some(false)),
          )
          ->ignore
        | _ => ()
        }
      }}>
      {React.string(
         switch (isLoggedIn) {
         | Some(true) => "Log out of GitHub"
         | Some(false) => "Login to GitHub"
         | None => "Checking your login status..."
         },
       )}
    </button>
  </div>;
};

let default = make;
