# Module and SDK API

* Proposal: [0026](0026-module-and-sdk-api/00-header.md)
* Authors: [Phoebe Goldman](https://github.com/gefjon), [Jeremie Pelletier](https://github.com/lcodes), [Ingvar Stepanyan](https://github.com/RReverser)
* Status: **Partially Implemented**
* Previous Proposals
  * [0005: Timers](../0005-timers.md)
  * [0006: Module RNG](../0006-module-rng.md)
  * [0008: Filtering Rules](../0008-filtering-rules.md)
  * [0009: Private Reducer Arguments](../0009-private-reducer-arguments.md)
  * [0012: Physical Access Bindings](../0012-physical-access-bindings.md)
  * [0022: Macro design](../0022-macro-design.md)
  * [0015: Subscription API](../0015-subscription-api.md)
  * [00??: WASM ABI stability](../0000-module-wasm-abi.md)

## Notes on structure

This proposal is larger and further-reaching than any we have written before (though not necessarily more technically complex), and as such the authors have elected to divide it into multiple files within this directory.

These are:

* [00 Header](./00-header.md): this file. Goals, non-goals, constraints and outline.
* [01 `DbConnection`](./01-db-connection.md): describes the `DbConnection` type, which mediates all accesses to the database.
* [02 Physical Queries](./02-physical-queries.md): describes methods for accessing tables in modules and the client cache in clients.
* [03 SDK Callbacks](./03-sdk-callbacks.md): describes the `EventContext` callback argument, which is provided to callbacks in the SDKs.
* [04: Table and Type Definitions](./04-table-and-type-definitions.md): describes surface syntax and user-facing semantics for `SpacetimeType` and table definitions, and row callbacks in clients.
* [05 User-Defined Reducers](./05-user-defined-reducers.md): describes interfaces related to defining reducers in modules, and requesting their invocation and registering reducer callbacks in clients.
* [06 Built-in Reducers](./06-built-in-reducers.md): describes builtin reducers which run in response to certain module events, and how SDKs listen for them.
* [07 Subscriptions](./07-subscriptions.md): describes user-facing syntax and semantics for subscriptions.
* [08 Credential Management](./08-credential-management.md): describes how SpacetimeDB actors (modules and clients) can read their own `Identity` and `Address`, and how clients can use the SDK to save them to persistent storage.
* [09 Module RNG](./09-module-rng.md): describes an interface for generating pseudo-random numbers in modules.
* [10 Module Timing Spans](./10-module-timing-spans.md): describes an interface for timing execution in modules.
* [11 SDK Event Loop](./11-sdk-event-loop.md): describes how the SDK advances and processes events.
* [12 Logger](./12-logger.md): describes how the SDK should expose logging capabilities.

## Motivation & Goals

When our software hits 1.0, we will no longer be able to make breaking changes to our APIs because customers will depend on them. This means that the rapidly-closing window before 1.0 is our last opportunity to revise our APIs' designs.

Our current module and SDK APIs expose a number of minor differences across the axes of context (module or SDK) and language (Rust, C# or TypeScript). These differences hinder users' developer experience, as code patterns which are valid in one context or language is invalid in another. They also impede SpacetimeDB development and maintenance, as team members familiar with one (context, language) point are unable to easily transition to working at another point.

Furthermore, our current APIs are in many ways untidy and brittle, owing largely to their history of extensions and reimplementations by a variety of team members without a shared design goal over a long period of time. Worse, lack of shared feature goals and changing priorities have left us with APIs which, in their current form, cannot be compatibly extended to expose new features we plan to add.

This proposal specifies APIs to which we will be committing for the 1.0 release for both modules and SDKs. Differences between the two, where necessary, are called out explicitly. Code is presented in Rust because the author feels most comfortable in that language, but is intended to be trivially translateable to at least C# and TypeScript. Any proposed interfaces which do not easily translate should be rejected during review and revised.

## Non-goals

### Idiomatic code in our variety of languages and contexts

At this stage, we are prioritizing consistency between module and client interfaces and between Rust, C# and TypeScript. This necessarily means that we will define unidiomatic interfaces which misuse or ignore unique features of the language, as such interfaces would not be directly translatable into other languages.

We prioritize consistency in this way to ease the barriers of onboarding and context-switching for users. We expect it to be common for a single developer or small team to implement e.g. a module in Rust, a desktop client in C# with Unity and a web client with TypeScript. Our intent is that such a developer or team should be easily able to switch between these contexts and languages while enjoying a familiar SpacetimeDB interface.

### `async` SDK API.

Designing and implementing an `async` API where SDK code operates directly on streams of events rather than registering callbacks is desirable, but out of scope for this proposal. Such an SDK API would necessarily not have a module counterpart, and significant design work is required to determine to what extent it can be portable between different SDK languages.

## Proposed solution

### Current functionality which must be maintained

#### Shared

- Iteration and filtering of a table in the TX state or client cache.
  - Incl. filtering by unique, indexed or unindexed columns.
  - This does not necessarily imply client-side indexes, which are currently implemented only in C#.
- Observing client connect and disconnect events.
  - I.e. `on_connect`, `on_disconnect` in modules; reducer callbacks for same in SDKs.
  - Currently improperly and inconsisently implemented in SDKs.
- Reading the current actor's `Identity` and `Address`.
  - Currently implemented inconsistently in SDKs and not (usefully) in modules.

#### Modules

- Insertion to and deletion from the TX state.
  - Incl. deleting a specific row, deleting by a value of an indexed column, deleting all rows of a table, and "updating" a row by a unique column (i.e. deleting the previous row and inserting a new row).
  - Not meaningful in SDKs: the client cache is an immutable mirror of the TX state.
- Running code on module initialization or upgrade, i.e. `init` and `update` reducers.
  - Not meaningful in SDKs: clients can run their own code whenever they want.
- Scheduling reducers to run at a later time. See [Proposal: Timers](../0005-timers.md).
  - Not meaningful in SDKs: clients must request scheduling via another reducer.
- Pseudo-Random Number Generation. See [Proposal: Module RNG](../0006-module-rng.md).
  - Not meaningful in SDKs: clients can access normal ecosystem-appropriate RNG without SpacetimeDB's intervention.
- Timing spans.
  - Not meaningful in SDKs: clients can access normal ecosystem-appropriate timing and benchmarking tools without SpacetimeDB's intervention.

#### SDKs

Note that many (possibly all) below items listed as not meaningful in modules will become meaningful in modules with the advent of inter-module communication.

- Credential management.
  - Not meaningful in modules: a database has a single identity and address known by the host which cannot be changed without external involvement.
- Managing connections to remote databases.
  - Initiating connections to remote databases with or without existing credentials.
  - Closing connections to remote databases.
  - Detecting abnormal disconnects from remote databases.
  - Future work will port the same interface to modules for inter-module communication.
- Asynchronously invoking reducers on remote databases.
  - Not meaningful in modules: modules can run their own reducers synchronously as normal functions, or schedule them to run in the future via the timers interface.
- Observing the status of reducer runs.
  - I.e. reducer callbacks and `ReducerEvent` arguments to row callbacks.
  - Incl. `on_connect`, `on_disconnect`, which is currently improperly and inconsisently implemented in SDKs.
  - Incl. `update`, which is currently not implemented in SDKs.
  - `init` not necessary, as no WebSocket connection can exist until after `init` commits.
  - Not meaningful in modules: modules only have control during reducers, not after or around them.
- Subscribing to queries. See [Proposal: Subscription Manipulation](https://github.com/clockworklabs/SpacetimeDBPrivate/blob/phoebe/proposal-multiple-subscriptions/proposals/0000-subscription-manipulation.md).
  - Future work will port the same interface to modules for inter-module communication.
- Observing modifications to subscribed rows.
  - I.e. row callbacks.
  - Incl. by non-reducer TXes, e.g. SQL queries via HTTP.
  - Future work may extend the same interface to modules for triggers.

### New interfaces or significant modifications

- Explicit `DbConnection` parameter to reducers and client callbacks through which all accesses to the database must pass.
  - Necessary in clients to allow multiple connections.
  - Will become necessary in modules to allow partitioning and inter-module communication.
- `count() -> u64` method for tables to return the number of rows in the TX state.
- `ReducerContext` fields renamed for clarity; prevent exhaustive matching.

## API and ABI Compatibility

This proposal, very intentionally, breaks both the module and SDK APIs. Users will have to rewrite their code in potentially significant and labor-intensive ways. We should consider writing a guide or FAQ describing how to rewrite old code to suit the new interfaces.

This proposal does not affect any ABIs or internal APIs in any way. Any new or modified user-facing interface which requires internal changes should be noted and added to the stability proposal for the relevant interface.
